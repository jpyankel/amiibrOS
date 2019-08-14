# slideshow
Generic slideshow app. Reads a configuration file and performs a looping
slideshow, displaying images or animated spritesheets to the screen with
various effects.

The resources folder must contain a configuration file named config.txt which
must be structured as follows:
* Every option setting is separated by a newline.
* Lines take the form: `<option> <settings list separated by space>`
* Every new slide has its `title` first before any of its options. In this way,
  the title of a slide (even if NULL or duplicate) is used as a separator in
  the config file so as to not confuse the parser.
* Similarly, every new image belonging to the slide has its attributes
  immediately following the `image_name` option. The `image_name` and `title`
  options act as separators between images.

If, at parsing time, the configuration file in the same directory as the
executable contains malformed configurations for a slide, the parser will
print the error to STDOUT and leave the malformed slide out of the slide loop.

There are no rules concerning whitespace - it would be wise to indent options
via tabs or spaces similarly to how other configuration formats (html, xml,
etc.) look for readibility. The only separator between options is a newline. If
you wish to escape a newline, use the `\` character at the end of a line; this
will cause the current line and the next line to be both parsed as one (not 
including the `\` of course).

The options, their descriptions, and example settings are as follows:
title <string>
* Modifies the title text. Leave empty (e.g. `title`) to hide title.
* title Example Title
title_duration <float>
* How long the title stays visible on the screen before fading out. 
* title_duration 4.0
slide_duration <float>
* How long the slide stays visible on the screen before the slide show moves
  onto the next slide.
* slide_duration 10.0
image_name <string>
* Specifies the filename of the image found in the resources folder (relative
to this folder).
* image_name josephImages/joe.png
* The previous examples assumes slideshow is being run in a directory
  containing slideshow and resources/, where resources contains some folders
  or files including josephImages/joe.png
tint_i
tint_f
tint_interp
tint_duration
pos_i (x, y)
* Modifies the initial position of the image.
* pos_i (112, 800)
pos_f (x, y)
* Sets the final position of the image to be used if animating.
* pos_f (130, 130)
pos_interp
pos_duration
size_i
size_f
size_interp
size_duration
rot_i
rot_f
rot_interp
rot_duration
