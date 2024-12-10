# Contributing

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit https://cla.microsoft.com.

When you submit a pull request, a CLA-bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., label, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

## Additional Helpful Links for Contributors

Many people all over the world have helped make this project better.  You'll want to check out:

- [What are some good first issues for new contributors to the repo?](https://github.com/azure/azure-sdk-for-c/issues?q=is%3Aopen+is%3Aissue+label%3Aup-for-grabs)
- [How to build and test your change](#developer-guide)
- [How you can make a change happen!](#pull-requests)
- Conceptual Topics in the detailed [Azure SDK for Embedded C docs](https://azure.github.io/azure-sdk-for-c).

## Community

- Chat with other community members [![Join the chat at https://gitter.im/azure/azure-sdk-for-c](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/azure/azure-sdk-for-c?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

## Reporting Security Issues and Security Bugs

Security issues and bugs should be reported privately, via email, to the Microsoft Security Response Center (MSRC) <secure@microsoft.com>. You should receive a response within 24 hours. If for some reason you do not, please follow up via email to ensure we received your original message. Further information, including the MSRC PGP key, can be found in the [Security TechCenter](https://www.microsoft.com/msrc/faqs-report-an-issue).

## How to Contribute to the Azure SDK for Embedded C

There are many ways that you can contribute to the Azure SDK for Embedded C project.

- For reporting bugs, requesting features, or asking for support, please file an issue in the [issues](https://github.com/Azure/azure-sdk-for-c/issues) section of the project.

- If you would like to become an active contributor to this project please follow the instructions provided in [Microsoft Azure Projects Contribution Guidelines](https://opensource.microsoft.com/collaborate).

- To make code changes, or contribute something new, please follow the [GitHub Forks / Pull requests model](https://docs.github.com/articles/fork-a-repo/): Fork the repo, make the change and propose it back by submitting a pull request.

- Refer to the [wiki](https://github.com/Azure/azure-sdk-for-c/wiki) to learn about how Azure SDK for Embedded C generates lint checker, doxygen, and code coverage reports.

### Pull Requests

- **DO** submit all code changes via pull requests (PRs) rather than through a direct commit. PRs will be reviewed and potentially merged by the repo maintainers after a peer review that includes at least one maintainer.
- **DO NOT** submit "work in progress" PRs.  A PR should only be submitted when it is considered ready for review and subsequent merging by the contributor.
- **DO** give PRs short-but-descriptive names (e.g. "Improve code coverage for Azure.Core by 10%", not "Fix #1234")
- **DO** refer to any relevant issues and include [keywords](https://docs.github.com/articles/closing-issues-via-commit-messages/) that automatically close issues when the PR is merged.
- **DO** tag any users that should know about and/or review the change.
- **DO** ensure each commit successfully builds.  The entire PR must pass all tests in the Continuous Integration (CI) system before it'll be merged.
- **DO** address PR feedback in an additional commit(s) rather than amending the existing commits, and only rebase/squash them when necessary.  This makes it easier for reviewers to track changes.
- **DO** assume that ["Squash and Merge"](https://github.com/blog/2141-squash-your-commits) will be used to merge your commit unless you request otherwise in the PR.
- **DO NOT** fix merge conflicts using a merge commit. Prefer `git rebase`.
- **DO NOT** mix independent, unrelated changes in one PR. Separate real product/test code changes from larger code formatting/dead code removal changes. Separate unrelated fixes into separate PRs, especially if they are in different assemblies.

#### Merging Pull Requests (for project contributors with write access)

- **DO** use ["Squash and Merge"](https://github.com/blog/2141-squash-your-commits) by default for individual contributions unless requested by the PR author.
  Do so, even if the PR contains only one commit. It creates a simpler history than "Create a Merge Commit".
  Reasons that PR authors may request "Merge and Commit" may include (but are not limited to):

  - The change is easier to understand as a series of focused commits. Each commit in the series must be buildable so as not to break `git bisect`.
  - Contributor is using an e-mail address other than the primary GitHub address and wants that preserved in the history. Contributor must be willing to squash
    the commits manually before acceptance.

### Developer Guide

#### Prerequisites

- [CMake](https://cmake.org/download/) version 3.10 or later
- C compiler: [MSVC](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2019), [gcc](https://gcc.gnu.org/) or [clang](https://clang.llvm.org/) are recommended
- [git](https://git-scm.com/downloads) to clone our Azure SDK repository with the desired tag
- [cmocka](https://cmocka.org/) for building and running unit tests. By default, building unit tests is disabled, so, unless you want to add unit tests or run then, you don't need to install this.
- [libcurl](https://curl.haxx.se/download.html) which is used as an http stack. You don't need to install libcurl if you are not building samples, or if you will provide another HTTP stack implementation. The minimum required version of libcurl is 7.1.

> Note: Using libcurl requires a global init and clean up that needs to happen in application code. See more info in Running Samples section.

- [doxygen](https://www.doxygen.nl/download.html) if you need to generate and view documentation.
- [clang-format](https://releases.llvm.org/download.html#9.0.0) to format the code properly. Note that you NEED `clang-format` from Clang version 9.0.0. Subsequent versions format code differently and we settled on this one for consistency. If you download the pre-built binaries version, it should be located at `<expanded clang dir>/bin/clang-format`.

### Running Tests

#### Unit Tests

See [CMake options][azure_sdk_for_c_cmake_options] to learn about how to build and run unit tests.

After compiling project with unit test enabled, run tests with:

```bash
cmake -DUNIT_TESTING=ON ..
cmake --build .
# ctest will call and run tests
# -V runs tests in verbose mode to show more info about tests
ctest -V
```

#### Test with Mocked Functions

Some test uses linker option ld to wrap functions and mock the implementation for it to do unit testing. Specially for PAL-related functions, mocking functions becomes a convenient way to break dependency between functions.

In order to run this tests, GCC is required (or any compiler that supports -ld linker flag).

To enable building project and linking with this option, as well as adding tests using mocked functions, add option `-DUNIT_TESTING_MOCKS=ON` next to `-DUNIT_TESTING=ON` to cmake cache generation (see below example)

```cmake
cmake -DUNIT_TESTING=ON -DUNIT_TESTING_MOCKS=ON ..
```

### Fix Code Formatting

Run the following command from the root of the sdk with `clang-format` version 9.0.0.

```bash
find . \( -iname '*.h' -o -iname '*.c' \) -exec clang-format -i {} \; 
```

Commit the resulting code formatting changes if there are any.

### Build Docs

Running below command from root folder will create a new folder `docs` containing html file with documentation about CORE headers. Make sure you have `doxygen` version *1.8.18* or later installed on the system.


```bash
doxygen doc/Doxyfile
```

### Code Coverage Reports

Code coverage reports can be generated after running unit tests for each project. Follow below instructions will generate code coverage reports.

#### Requirements

- **gcc** - clang/MSVC are not supported
- **Debug** - Build files for debug `cmake -DCMAKE_BUILD_TYPE=Debug ..`
- **cmocka / Unit Test Enabled** - Build cmocka unit tests `cmake --DUNIT_TESTING=ON ..`
- **environment variable** - `set AZ_SDK_CODE_COV=1`

```bash
# from source code root, create a new folder to build project:
mkdir build
cd build

# set env variable to enable building code coverage
export AZ_SDK_CODE_COV=1
# generate cmake files with Debug and cmocka unit tests enabled
cmake -DUNIT_TESTING=ON -DCMAKE_BUILD_TYPE=Debug ..
# build
cmake --build .

## There are 3 available reports to generate for each project:
# 1. using lcov. Html files grouped by folders. Make sure lcov
# is installed.
make ${project_name}_cov //i.e. az_core_cov or az_iot_cov

# 2. using gcov. Html page with all results in one page. Make sure
# gcov is installed.
make ${project_name}_cov_html //i.e. az_core_cov_html or az_iot_cov_html

# 3. using gcov. XML file with all results. Make sure
# gcov is installed.
make ${project_name}_cov_xml //i.e. az_core_cov_xml or az_iot_cov_xml

## Code Coverage is available for these projects:
#  az_core
#  az_iot

> Note: If `make` fails with "project not found" it's likely you are not using `gcc`. Use `sudo update-alternatives --config c+++` and `sudo update-alternatives --config cc` to switch to gcc.
```

<!-- LINKS -->
[azure_sdk_for_c_cmake_options]: https://github.com/Azure/azure-sdk-for-c#cmake-options
