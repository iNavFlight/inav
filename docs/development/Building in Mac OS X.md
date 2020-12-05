# Building in Mac OS X

Building in Mac OS X can be accomplished in just a few steps:

* Install general development tools (clang, make, git)
* Install cmake
* Checkout INAV sourcecode through git
* Build the code

## Install general development tools (clang, make, git)

Open up a terminal and run `make`. If it is installed already, you should see a message like this, which means that you
already have the required development tools installed:

```
make: *** No targets specified and no makefile found.  Stop.
```

If it isn't installed yet, you might get a popup like this. If so, click the "install" button to install the commandline
developer tools:

![Prompt to install developer tools](assets/mac-prompt-tools-install.png)

If you just get an error like this instead of a helpful popup prompt:

```
-bash: make: command not found
```

Try running `xcode-select --install` instead to trigger the popup.

If that doesn't work, you'll need to install the XCode development environment [from the App Store][]. After
installation, open up XCode and enter its preferences menu. Go to the "downloads" tab and install the
"command line tools" package.

[from the App Store]: https://itunes.apple.com/us/app/xcode/id497799835

## Install cmake

The easiest way to install cmake's command line executable is via
[Homebrew](https://brew.sh) (a package manager for macOS). Go to their site
and follow their installation instructions.

Once Homebrew is installed, type `brew install cmake` in a terminal to install
cmake.

Alternatively, cmake binaries for macOS are available from
[cmake.org](https://cmake.org/download/). If you prefer installing it this way,
you'd have to manually add cmake's command line binary to your `$PATH`. Assuming
`CMake.app` has been copied to `/Applications`, adding the following line to
`~/.zshrc` would make the cmake command available.

```sh
export PATH=$PATH:/Applications/CMake.app/Contents/bin
```

## Ruby

Ruby is installed by default on macOS.

## Checkout INAV sourcecode through git

Enter your development directory and clone the [INAV repository][] using the "HTTPS clone URL" which is shown on
the right side of the INAV GitHub page, like so:

```
git clone https://github.com/iNavFlight/inav
```

This will download the entire INAV repository for you into a new folder called "inav".

[INAV repository]: https://github.com/iNavFlight/inav.git

## Build the code

Assuming you've just cloned the source code, you can switch your current
directory to inav's source try by typing:

```sh
cd inav
```

Inside the inav directory, create a new directory to store the built files. This
helps keeping everything nice and tidy, separating source code from artifacts. By
convention this directory is usually called `build`, but any name would work. Enter
the following command to create it and switch your working directory to it:

```sh
mkdir -p build && cd build
```

Now we need to configure the build by using the following command:

```sh
cmake ..
```

This will automatically download the required compiler for inav, so it
might take a few minutes. Once it's finished without errors, you can
build the target that you want by typing `make target-name`. e.g.:

```sh
make -j8 MATEKF722 # Will build MATEKF722 target
```

A list of all the available targets can be displayed with:

```sh
make targets
```

Once the build completes, the correspondent `.hex` file will be found
in current directory (e.g. `build`) and it will be named as
`inav_x.y.z_TARGET.hex`. `x.y.z` corresponds to the INAV version number
while `TARGET` will be the target name you've just built. e.g.
`inav_2.6.0_MATEKF722.hex`. This is the file that can be flashed using
INAV Configurator.

## Updating to the latest source

If you want to erase your local changes and update to the latest version of the INAV source, enter your
inav directory and run these commands to first erase your local changes, fetch and merge the latest
changes from the repository, then rebuild the firmware:

```sh
git reset --hard
git pull

make target-name # e.g. make MATEKF722
```
