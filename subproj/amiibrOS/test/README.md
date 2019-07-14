# Testing amiibrOS on Linux Host
TODO

## Setup Instructions

Remember that `make test` must be run in the parent directory just above this
README in order to compile the source code and auto-copy the resources folder.

Once the previous step is done, two commands need to be run:
`sudo mkdir /usr/bin/amiibrOS`
`sudo cp -r . /usr/bin/amiibrOS`

Finally, to test amiibrOS `cd /usr/bin/amiibrOS` and run the executable there
called `amiibrOS_test`.

## Adding Test Programs

To add testing programs, see the `app/00000000` example. All apps must live in
a folder sharing its 4-byte name. If, for instance, the scanner reads the 4-
byte sequence `00000000`, then amiibrOS will run the shell script
`/usr/bin/amiibrOS/app/00000000/00000000.sh`. This shell script should be
edited to run whatever test (python script or compiled c program) you specify.
