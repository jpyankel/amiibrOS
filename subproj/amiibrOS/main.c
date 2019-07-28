/**
 * main.c
 *
 * Contains implementation of os_ctrl.
 *
 * Spawns an amiibo_scan.py process and main_interface process upon starting.
 * 
 * Continuously monitors a pipe written to by the amiibo_scan process to allow
 *   for switching between game/UI processes.
 *
 * Terminates execution and outputs an error message when any programmer caused
 *   errors occur. This is to help find bugs; the goal is to never have the
 *   program terminate upon release. Recoverable errors aside from these will
 *   always be recovered from (see read_tag_info for signal example).
 * 
 * Joseph Yankel (jpyankel@gmail.com)
 */

#include <unistd.h> // pipe, fork, execv, ... etc. system calls.
#include <stdio.h> // perror, sprintf
#include <errno.h> // errno
#include <stdlib.h> // exit
#include <signal.h> // signal
#include <sys/wait.h> // waitpid, wait
#include <sys/stat.h> // stat
#include <stdbool.h> // true, false
#include <pthread.h> // various multithreading
#include "interface.h" // amiibrOS interface

#define INTERPRETER_PATH "/usr/bin/python"
#define A_SCAN_PATH "/usr/bin/amiibrOS/amiibo_scan/amiibo_scan.py"

// Raw info size in bytes to be retrieved from scanner output:
#define RAW_INFO_SIZE 4
// Size of hex string we will construct from the raw info (not including NUL):
#define HEX_TAG_SIZE (RAW_INFO_SIZE*2)

// Directory holding all of the game/display app directories:
#define APP_ROOT_PATH "/usr/bin/amiibrOS/app"
// Length of "/usr/bin/amiibrOS/app/12345678" (+1 for extra '/', no NUL)
#define APP_DIR_LEN (sizeof(APP_ROOT_PATH) + HEX_TAG_SIZE + 1)
// Length of "/usr/bin/amiibrOS/app/12345678/12345678" (+1 for '/', no NUL)
#define APP_PATH_LEN (APP_DIR_LEN + HEX_TAG_SIZE + 1)

// Error message for when the scanner terminates early:
#define SIGCHLD_SCANNER_ERROR "os_ctrl unexpected sigchld\nerror: "\
                              "sigchld_handler reaped scanner\n"
// -1 since we need not include the NUL terminator:
#define SIGCHLD_SCANNER_ERROR_LEN (sizeof(SIGCHLD_SCANNER_ERROR) - 1)
// Error message for when a programmer error occurs:
#define PROG_ERROR "os_ctrl programmer error occured\n"
// Length of above error:
#define PROG_ERROR_LEN (sizeof(PROG_ERROR) - 1)

// amiibo scan subprocess pid. Important for sigchld_handler. Should be set
//   only once during this process's lifetime.
static pid_t a_scan_pid;
static pid_t app_pid; // current game/display pid
static bool app_pid_set = false; // has app_pid been set at least once?
static int pipefds[2]; // pipes to communicate with scanner program

/**
 * Prints error message (and optionally errno's error).
 * Sends SIGTERM to all child processes and waits for each to exit.
 * Terminates the program with an error status.
 * This is not async signal safe.
 *
 * Should be called from parent process
 */
void p_exit_err (const char *msg, bool perrno)
{
  if (perrno) 
    perror(msg);
  else
    printf(msg);

  // Ignore sigchld so that we don't jump to sigchld_handler:
  sigset_t block_set;
  sigemptyset(&block_set);
  sigaddset(&block_set, SIGCHLD);
  sigprocmask(SIG_BLOCK, &block_set, NULL);

  // Send terminate signal to all inside our process group
  pid_t gid = getpgid(getpid());
  kill(-gid, SIGTERM);

  while (wait(NULL) > 0); // Will wait until -1 returned (ERROR)
  // We just assume that the ERROR is no child processes left, ignore any other
  //   wait errors, just return:
  exit(1);
}

/**
 * Prints error message.
 * Sends SIGTERM to all child processes and waits for each to exit.
 * Terminates the program with an error status.
 * This version is async-signal-safe.
 *
 * Should be called from parent process.
 */
void p_exit_err_sigsafe (const char *msg, size_t len)
{
  // Ignore sigchld so that we don't jump to sigchld_handler:
  sigset_t block_set;
  sigemptyset(&block_set);
  sigaddset(&block_set, SIGCHLD);
  sigprocmask(SIG_BLOCK, &block_set, NULL);

  // Print error message:
  write(STDOUT_FILENO, msg, len);
  // Send terminate signal to all children:
  pid_t gid = getpgid(getpid());
  kill(-gid, SIGTERM);
  // Wait until all finish:
  while (wait(NULL) > 0);

  exit(1);
}

/**
 * Send SIGTERM signal to parent process after reporting error.
 * Should be called from child process.
 */
void c_exit_err (const char *msg, bool perrno)
{
  if (perrno) 
    perror(msg);
  else
    printf(msg);

  kill(getppid(), SIGTERM); // send terminate signal to parent process.
  exit(1);
}

/**
 * SIGCHLD handler. It is used to reap app subprocesses and to detect if the
 *   scanner subprocess terminates.
 *
 * If the a_scan_pid process was reaped, then an error is thrown and the
 *   program is forced to terminate.
 */
void sigchld_handler(int sig)
{
  (void)sig; // Ignore compiler warnings
  int preserve_errno = errno;
  
  pid_t p;
  // Quickly check and reap ready processes (do not block if there are none)
  while ( (p = waitpid(-1, NULL, WNOHANG)) > 0) {
    if (p == a_scan_pid) {
      // print error message, wait for processes to die, exit.
      p_exit_err_sigsafe(SIGCHLD_SCANNER_ERROR, SIGCHLD_SCANNER_ERROR_LEN);
    }
  }
  if (p == -1) {
    // A programmer error occured.
    p_exit_err_sigsafe(PROG_ERROR, PROG_ERROR_LEN);
  }

  errno = preserve_errno;
}

/**
 * Handles SIGTERM (and SIGINT) signals.
 * Signals SIGTERM to all children and waits to reaps them all before exiting.
 */
void sigterm_handler(int sig)
{
  (void)sig; // Ignore compiler warning

  // Ignore sigchld so that we don't jump to sigchld_handler:
  sigset_t block_set;
  sigemptyset(&block_set);
  sigaddset(&block_set, SIGCHLD);
  sigprocmask(SIG_BLOCK, &block_set, NULL);

  // Send terminate signal to all inside our process group
  pid_t gid = getpgid(getpid());
  kill(-gid, SIGTERM);

  while (wait(NULL) > 0); // Will wait until all child processes terminate
  
  exit(1);
}

/**
 * Reads RAW_INFO_SIZE bytes from pipe read end given by pipefd and stores it
 *   in info_buf. Returns the total number of bytes read - useful for checking
 *   if the write-end of the pipe was closed prematurely.
 *
 * Accounts for signal interruptions that may occur by restarting the read.
 *   If read would fail in any other case, this function causes the program to
 *   with an error message.
 *
 * Note, this is a blocking call. We only return when RAW_INFO_SIZE bytes have
 *   been read into info_buf, or if the write-end of the pipe was closed.
 */
size_t read_raw_info (int pipefd, char *raw_info)
{
  ssize_t rd_total = 0; // # of chars in info_buf (also next idx to write to)
  ssize_t rd_cnt; // tmp var for storing return value of read sys call

  // read will block until either EOF, error, or some number of chars are read
  // We must also check if rd_total has already been read
  while ( rd_total != RAW_INFO_SIZE &&
      (rd_cnt = read(pipefd, raw_info+rd_total, RAW_INFO_SIZE-rd_total)) != 0)
  {
    // We have read a non-zero number of chars!
    if (rd_cnt == -1) { // Check for errors
      if (errno != EINTR) { // If our read wasn't interrupted by a signal...
        // We have a serious error:
        perror("os_ctrl pipe read failed\nerror");
        exit(1);
      }
      // Else, we just restart the read.
    }
    else {
      // On the next pipe read, we write into info_buf starting from rd_total
      rd_total += rd_cnt; // ... so we need to update the total at each read
    }
  }

  return rd_total;
}

/**
 * Converts a char array containing raw bytes into a hex string.
 * Array hex_tag must be of length HEX_TAG_SIZE + 1
 */
void raw_to_hex_tag (const char *raw_info, char *hex_tag)
{
  for (unsigned int i = 0; i < RAW_INFO_SIZE; i++) {
    sprintf(hex_tag + 2*i, "%02X", (unsigned char)raw_info[i]);
  }
  hex_tag[HEX_TAG_SIZE] = '\0'; // Assign NUL manually
}

/**
 * Uses the given hex_tag to find an app for launching.
 * Then tells the UI thread to play an animation and blocks until the animation
 *   is complete.
 * Launches this app, replacing any old app processes.
 */
void launch_app (const char *hex_tag)
{
  char app_dir[APP_DIR_LEN + 1]; // +1 for NUL
  char app_path[APP_PATH_LEN + 4]; // +1 for NUL +3 for ".sh"
  struct stat stat_buf; // We need this as a mandatory parameter for stat call

  // Construct the paths to the directory and executable:
  sprintf(app_dir, "%s/%s", APP_ROOT_PATH, hex_tag);
  sprintf(app_path, "%s/%s.sh", app_dir, hex_tag);

  printf("app_path: %s\n", app_path); // TODO REMOVE

  // Check if a program matching hex_tag exists and is accessible:
  if (stat(app_path, &stat_buf) != -1) {
    // Tell our UI to play 'amiibo scanned' and 'fade out' animation and then
    //   auto stop:
    play_scan_anim(true); // Blocks until animation completes
    fade_out_interface(); // Blocks until animation completes. Kills UI thread.

    // Send SIGTERM signal to current app_pid (if exists) to close it:
    if (app_pid_set)
      kill(app_pid, SIGTERM);
    // Reaping is done via the sigchld handler

    // Attempt to execute a new app:
    if ( (app_pid = fork()) == 0) {
      // Close unused read-end of pipe for the newly spawned app
      if (close(pipefds[0]))
        c_exit_err("os_ctrl unable to close write end of pipe\nerror", true);

      // Change directory to exec from (we want all relative paths to function)
      if (chdir(app_dir) == -1)
        c_exit_err("os_ctrl unable to change dir. for new app\nerror", true);

      execl("/bin/sh", "sh", app_path, NULL); // Execute as .sh shell script
      
      c_exit_err("os_ctrl unable to spawn app\nerror", true);
      // Note, as mentioned ealier, no need to undo our signal stuff
    }
    app_pid_set = true;
  }
  else {
    // No program matches. Notify user of the given amiibo's incompatibility:
    // Tell UI to play 'not found' animation.
    play_scan_anim(false);
    perror("AMIIBO APP NOT FOUND\nerror"); // TODO Remove
  }
}

int main (void)
{
  sigset_t prev_set, block_set;

  // Set up pipe for communication between amiibo_scan and this process
  if (pipe(pipefds))
    p_exit_err("os_ctrl unable to create pipe\nerror", true);

  // Construct signal block mask:
  if (sigemptyset(&block_set) == -1 || sigaddset(&block_set, SIGCHLD) == -1)
    p_exit_err("os_ctrl unable to create signal mask\nerror", true);

  // Block signals:
  if (sigprocmask(SIG_BLOCK, &block_set, &prev_set) == -1)
    p_exit_err("os_ctrl unable to block signals\nerror", true);

  // Install SIGCHLD handler.
  if (signal(SIGCHLD, sigchld_handler) == SIG_ERR)
    p_exit_err("os_ctrl unable to install signal handler\nerror", true);
  
  // Install SIGTERM and SIGINT handler:
  if (signal(SIGTERM, sigterm_handler) == SIG_ERR)
    p_exit_err("os_ctrl unable to install signal handler\nerror", true);
  if (signal(SIGINT, sigterm_handler) == SIG_ERR)
    p_exit_err("os_ctrl unable to install signal handler\nerror", true);

  if ( (a_scan_pid = fork()) == 0) { // SCANNER CHILD BEGIN
    // Delete unused read-end of the pipe for child
    if (close(pipefds[0]))
      c_exit_err("os_ctrl unable to close read end of pipe\nerror", true);

    // Execute amiibo_scan.py
    // Construct argv and exec the python interpreter:
    char fd_str[12];
    sprintf(fd_str, "%d", pipefds[1]); // We want to send the write-end of pipe
    char *const argv[] = {INTERPRETER_PATH, A_SCAN_PATH, fd_str, NULL};
    execv(INTERPRETER_PATH, argv);

    // If execv returns, we had an error
    perror("os_ctrl unable to spawn amiibo_scan\nerror");
    // Note: We did not need to fiddle with the signal handler or blocked set
    //   in the child process as they get overwritten by execv anyways.
  } // SCANNER CHILD END
  else {
    // Close unused write-end of pipe for parent (and for other process).
    if (close(pipefds[1]))
      p_exit_err("os_ctrl unable to close write end of pipe\nerror", true);

    // Start a new thread for our main interface:
    pthread_t uithread;
    if (pthread_create(&uithread, NULL, start_interface, NULL)) {
      p_exit_err("amiibrOS unable to start UI thread\nerror", true);
    }
    // Note that when main exits, the system will automatically kill uithread.

    // Continuously monitor the scanner:
    char raw_info[RAW_INFO_SIZE];
    char hex_tag[HEX_TAG_SIZE + 1]; // +1 for NUL
    for(;;) {
      // Read raw tag
      if (read_raw_info(pipefds[0], raw_info) == 0) {
        // Write-end of pipe closed prematurely. There is an error!
        p_exit_err("os_ctrl detected erroneous pipe disconnect\nerror: pipe "
            "write-end closed prematurely\n", false);
      }

      // Convert raw tag to hex string:
      raw_to_hex_tag(raw_info, hex_tag);
      
      // Launch app based on hex string:
      launch_app(hex_tag);
    }
  }
  
  return 0;
}
