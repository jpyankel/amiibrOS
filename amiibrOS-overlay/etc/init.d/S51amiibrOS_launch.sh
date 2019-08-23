#! /bin/sh
# /etc/init.d/amiibrOS_launch.sh
#
# description: amiibrOS init script
# processname: amiibrOS_init
#
# Joseph Yankel (jpyankel)

start() {
  echo "starting amiibrOS..."
  cd /usr/bin/amiibrOS && ./amiibrOS &
}

case "$1" in
    start)
  start
  ;;
    *)
  echo "Usage: $0 {start}"
	exit 1
  ;;
esac

exit 0
