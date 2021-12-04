## About The Project
FileHasher is a command line utility, that provides possibility to split input file into blocks of a predefined size, calculate hash of each block and print hash values to the output file.
For better performance, hash calculation is performed in multiple threads.
 
### Built With
The utility was created with the help of the following libraries, frameworks, IDE-s:
* C++17
* [boost](https://www.boost.org/users/history/version_1_77_0.html)
* [Microsoft Visual Studio](https://visualstudio.microsoft.com/)
 
## Getting Started
To get a local copy up and running, follow the steps below.
 
### Prerequisites
Download and install Microsoft Visual Studio 2017 of version 15.9.39.
 
### Installation
1. Clone the repo
   ```sh
   git lfs clone https://github.com/AlexeyBudko/FileHasher
   ```
2. Open project FileHasher in Visual Studio and build it.
 
## Usage
The utility is configured via command line parameters. The following ones are supported:
- -\-help - provides utility and command line parameters description.
- -\-input - specify path to file which, for which hashes shall be calculated.
- -\-output - specify path to file, where calculated hashes shall be located.
- -\-size (optional. default value: 1Mb) - size of block in bytes, which is used to split input file.
 
## Contact
Alexey Budko - [@linkedin](https://www.linkedin.com/in/alexey-budko-8463b7146/) lesha_11@bk.ru

