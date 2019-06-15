# slideshow
Generic slideshow app. Reads a configuration file and performs a looping
slideshow, displaying images or animated spritesheets to the screen with
various effects.

The resources folder must contain a configuration file named config.txt which
must be structured as follows:
* Every option setting is separated by a newline.
* Lines take the form: `<option> <settings list separated by space>`
* Every new slide has its title first before any of its options. In this way,
  the title of a slide (even if NULL or duplicate) is used as a separator in
  the config file so as to not confuse the parser.

The settings are as follows:
* TODO
