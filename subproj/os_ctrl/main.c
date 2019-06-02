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
#include <stdio.h> // perror
#include <errno.h> // errno
#include <stdlib.h> // exit
#include <signal.h> // signal
#include <sys/wait.h> // waitpid

#define INTERPRETER_PATH "/usr/bin/python"
#define A_SCAN_PATH "/usr/bin/amiibrOS/amiibo_scan/amiibo_scan.py"
#define MAIN_INTERFACE_FOLDER "/usr/bin/amiibrOS/main_interface"
#define MAIN_INTERFACE_PATH (MAIN_INTERFACE_FOLDER "/main_interface")

// Tag info size in bytes to be retrieved from scanner output:
#define TAG_INFO_SIZE 4

// Error message for when the scanner terminates early:
#define SIGCHLD_SCANNER_ERROR "os_ctrl unexpected sigchld\nerror: "\
                              "sigchld_handler reaped scanner\n"
// -1 since we need not include the NUL terminator:
#define SIGCHLD_SCANNER_ERROR_LEN sizeof(SIGCHLD_SCANNER_ERROR) - 1
// Error message for when a programmer error occurs:
#define PROG_ERROR "os_ctrl programmer error occured\n"
#define PROG_ERROR_LEN sizeof(PROG_ERROR) - 1

// amiibo scan subprocess pid. Important for sigchld_handler. Should be set
//   only once during this process's lifetime.
pid_t a_scan_pid;

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
      write(STDOUT_FILENO, SIGCHLD_SCANNER_ERROR, SIGCHLD_SCANNER_ERROR_LEN);
      exit(1);
    }
  }
  if (p == -1) {
    // A programmer error occured.
    write(STDOUT_FILENO, PROG_ERROR, PROG_ERROR_LEN);
    exit(1);
  }

  errno = preserve_errno;
}

/**
 * Reads TAG_INFO_SIZE bytes from pipe read end given by pipefd and stores it
 *   in info_buf. Returns the total number of bytes read - useful for checking
 *   if the write-end of the pipe was closed prematurely.
 *
 * Accounts for signal interruptions that may occur by restarting the read.
 *   If read would fail in any other case, this function causes the program to
 *   with an error message.
 *
 * Note, this is a blocking call. We only return when TAG_INFO_SIZE bytes have
 *   been read into info_buf, or if the write-end of the pipe was closed.
 */
size_t read_tag_info (int pipefd, char *info_buf)
{
  ssize_t rd_total = 0; // # of chars in info_buf (also next idx to write to)
  ssize_t rd_cnt; // tmp var for storing return value of read sys call

  // read will block until either EOF, error, or some number of chars are read
  // We must also check if rd_total has already been read
  while ( rd_total != TAG_INFO_SIZE &&
      (rd_cnt = read(pipefd, info_buf+rd_total, TAG_INFO_SIZE-rd_total)) != 0)
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

int main (void)
{
  int pipefds[2];
  pid_t app_pid; // current game/display pid
  sigset_t prev_set, block_set;

  // Set up pipe for communication between amiibo_scan and this process
  if (pipe(pipefds)) {
    perror("os_ctrl unable to create pipe\nerror");
    return 1; // Exit with error
  }

  // Construct signal block mask:
  if (sigemptyset(&block_set) == -1 || sigaddset(&block_set, SIGCHLD) == -1) {
    perror("os_ctrl unable to create signal mask\nerror");
    return 1;
  }
  // Block signals:
  if (sigprocmask(SIG_BLOCK, &block_set, &prev_set) == -1) {
    perror("os_ctrl unable to block signals\nerror");
    return 1;
  }
  // Install SIGCHLD handler.
  if (signal(SIGCHLD, sigchld_handler) == SIG_ERR) {
    printf("os_ctrl unable to install signal handler\nerror");
    return 1;
  }

  if ( (a_scan_pid = fork()) == 0) { // SCANNER CHILD BEGIN
    // Delete unused read-end of the pipe for child
    if (close(pipefds[0])) {
      perror("os_ctrl unable to close read end of pipe\nerror");
      return 1;
    }

    // Execute amiibo_scan.py
    // Construct argv and exec the python interpreter:
    char fd_str[12];
    sprintf(fd_str, "%d", pipefds[1]); // We want to send the write-end of pipe
    char *const argv[] = {INTERPRETER_PATH, A_SCAN_PATH, fd_str, NULL};
    execv(INTERPRETER_PATH, argv);

    // If execv returns, we had an error
    perror("os_ctrl unable to spawn amiibo_scan\nerror");
    return 1;
    // Note: We did not need to fiddle with the signal handler or blocked set
    //   in the child process as they get overwritten by execv anyways.
  } // SCANNER CHILD END
  else {
    // Close unused write-end of pipe for parent (and for other process).
    if (close(pipefds[1])) {
      perror("os_ctrl unable to close write end of pipe\nerror");
      return 1;
    }

    // Fork the main_interface process as our first app:
    if ( (app_pid = fork()) == 0) { // APP CHILD BEGIN
      // Close unused read-end of pipe for the newly spawned app
      if (close(pipefds[0])) {
        perror("os_ctrl unable to close write end of pipe\nerror");
        return 1;
      }

      // Change directory to exec from (we want all relative paths to function)
      if (chdir(MAIN_INTERFACE_FOLDER) == -1) {
        perror("os_ctrl unable to change directory for main_interface\nerror");
        return 1;
      }

      char *const argv[] = {MAIN_INTERFACE_PATH, NULL};
      execv(MAIN_INTERFACE_PATH, argv);
      
      perror("os_ctrl unable to spawn main_interface\nerror");
      return 1;
      // Note, as mentioned ealier, no need to undo our signal stuff
    } // APP CHILD END
    else {
      // Unblock SIGCHLD
      if (sigprocmask(SIG_SETMASK, &prev_set, NULL) == -1) {
        perror("os_ctrl unable to unblock signals\nerror");
        return 1;
      }
      // Continuously monitor the scanner:
      char info_buf[TAG_INFO_SIZE];
      for(;;) {
        if (read_tag_info(pipefds[0], info_buf) == 0) {
          // Write-end of pipe closed prematurely. There is an error!
          printf("os_ctrl detected erroneous pipe disconnect\nerror: pipe "
              "write-end closed prematurely\n");
          return 1;
        }

        // TODO -------------------- DELETE -------------------------
        printf("Parent read: ");
        for (size_t i = 0; i < TAG_INFO_SIZE; i++) {
          printf("%02X", (unsigned char)(info_buf[i]));
        }
        printf("\n");
        // TODO -------------------- DELETE -------------------------
        // Identify the amiibo scanned:
        // TODO
        // Send SIGTERM signal to current app_pid, reap it, and fork next app:
        // TODO
      }
    }
  }
  
  return 0;
}
