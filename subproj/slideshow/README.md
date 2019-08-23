# slideshow
Generic slideshow app. Reads a configuration file and performs a looping
slideshow, displaying images to the screen with various effects.

## Rules
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

## Configuration
The options, their descriptions, and example settings are as follows:

title <string>
* Modifies the title text. Leave empty (e.g. `title`) to hide title.
* `title Example Title`

title_duration <float>
* How long the title stays visible on the screen before fading out. 
* `title_duration 4.0`

slide_duration <float>
* How long the slide stays visible on the screen before the slide show moves
  onto the next slide.
* `slide_duration 10.0`

image_name <string>
* Specifies the filename of the image found in the resources folder (relative
to this folder).
* `image_name josephImages/joe.png`
* The previous examples assumes slideshow is being run in a directory
  containing slideshow and resources/, where resources contains some folders
  or files including josephImages/joe.png

tint_i <(0-255, 0-255, 0-255, 0-255)>
* Initial image tint.
* `tint_i (50, 255, 13, 255)`

tint_f
* Final image tint to be used if animating via tint_interp and tint_duration.
* `tint_f (50, 255, 13, 0)`

tint_interp <0-9>
* Which interpolation method should we use for animating the tint value between
tint_i and tint_f (see below section for interpolation types).
* `tint_interp 1`
* The above config will make the image tint interpolate linearly over
tint_duration amount of time (assuming tint_duration != 0).

tint_interp_captype <0-2>
* Which of EASE IN, EASE OUT, or BOTH to use for the chosen tint_interp. This
simply tweaks the animations behaviour at the start and end of its duration.
* `tint_interp_captype 1`

tint_duration <float>
* Over what duration of time to animate the image tint value via tint_interp.
* `tint_duration 10.5`
* The previous command will make the image tint interpolate using tint_interp
over 10.5 seconds.
* If tint_duration is set to 0, then the image will not animate.

pos_i (x, y)
* Modifies the initial position of the image.
* `pos_i (112, 800)`

pos_f (x, y)
* Sets the final position of the image to be used if animating.
* `pos_f (130, 130)`

pos_interp <0-9>
* Which interpolation method should we use for animating the position value
between pos_i and pos_f (see below section for interpolation types).
* `pos_interp 0`
* The above config will cause the position to not interpolate at all.

pos_interp_captype <0-2>
* Which of EASE IN, EASE OUT, or BOTH to use for the chosen pos_interp. This
simply tweaks the animations behaviour at the start and end of its duration.
* `pos_interp_captype 1`

pos_duration <float>
* Over what duration of time to animate the image's position using the
interpolation type specified by pos_interp.
* `pos_duration 5`
* The previous command will make the image position change over 5 seconds if
pos_interp is not 0.
* If pos_duration is set to 0, then the image position will not animate.

size_i (length, width)
* Modifies the initial size of the image. This is not a multiplier, but the
actual space in pixels the image will take up (before any upscaling).
* `size_i (400, 400)`

size_f (length, width)
* Modifies the final size of the image to be used when animating. This is not a
multiplier, but the actual space in pixels the image will take up (before any
upscaling).
* `size_f (1440, 900)`

size_interp <0-9>
* Which interpolation method should we use for animating the image's size
between size_i and size_f (see below section for interpolation types).
* `size_interp 4`

size_interp_captype <0-2>
* Which of EASE IN, EASE OUT, or BOTH to use for the chosen size_interp. This
simply tweaks the animations behaviour at the start and end of its duration.
* `size_interp_captype 2`

size_duration <float>
* Over what duration of time to animate the image's size using the
interpolation type specified by size_interp.
* `size_duration 12`
* If size_duration is set to 0, then the image size will not animate.

rot_i <float>
* The initial angle of the rectangle (in degrees).
* `rot_i 180`
* The previous command will flip the image around its origin point.

rot_f <float>
* The final angle to rotate to if animating.
* `rot_f 360`

rot_interp <0-9>
* Which interpolation method should we use for animating the rotation value
between rot_i and rot_f (see below section for interpolation types).
* `rot_interp 9`

rot_interp_captype <0-2>
* Which of EASE IN, EASE OUT, or BOTH to use for the chosen rot_interp. This
simply tweaks the animations behaviour at the start and end of its duration.
* `rot_interp_captype 2`

rot_duration <float>
* Over what duration of time to animate the image's rotation using the
interpolation type specified by rot_interp.
* `rot_duration 8`
* If rot_duration is set to 0, then the image will not animate its rotation.

## Interpolation Types
The interpolation types used and their codes are listed below:
* NONE = 0
* LINEAR = 1
* SINE = 2
* CIRCULAR = 3
* CUBIC = 4
* QUADRATIC = 5
* EXPONENTIAL = 6
* BACK = 7
* BOUNCE = 8
* ELASTIC = 9

## TODO
* Ability to add a looping soundtrack. Functionality can be added via Raylib.
* Want to add the ability to animate spritesheets. For this, we would need to
use Raylib's srcRec argument in DrawTexturePro (see main.c TODO comment in
main).
* Add support for title displaying. This was supposed to be a main feature, but
didn't make it in Summer 2019 due to time constraints (see TODO in main.c
main).
* Add support for changing the origin of the image to be displayed. Currently
the origin is always centered, which makes it more difficult to use for 
fullscreen images. See the tutorial on
[my website](https://jpyankel.github.io/2019/08/23/amiibrOS_slideshow_tutorial.html)
for how to position slideshow images where you want them in its current state.
* Proper testing on all of the configurations. I've only personally used a few,
and haven't tested them all yet.
