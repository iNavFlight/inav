# Building in Gitpod

Gitpod offers an online build environment for building INAV targets.
## Setting up the environment and building targets

1.  Go to https://gitpod.io/new
1.  Paste `https://github.com/iNavFlight/inav/tree/[version]` into the field called "Select a repository". 
1.  Ensure that you substitute [version] (e.g. 7.1.0) with the version number of INAV that you want to build.
1.  Cick on the link that shows in the drop down and Gitpod will atomatically selects the adequate Editor and Browser.
1.  Leave the other fields as default and click "Continue". Your build environment will be created.
1.  At the bottom of the page, you will see a command line. Type `make [TARGET]` and wait for the target to be built.
1.  Once the build has finished, navigate to the build folder using `cd build`.
1.  Once in the folder, run `objcopy -O ihex -R .eeprom [TARGET].elf [TARGET].hex` to convert the `.elf` file to a `.hex` file.
1.  Your new target `.hex` binary will be located in a folder called `bin`, which can be found at the top left of the page.


NOTE: You can use this method to build your forks as well. Just paste in the link to your fork and follow the rest of the steps.

You are done!
