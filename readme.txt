EDK II Build Data Viewer

Open source code located at https://github.com/01org/edkiibuilddataviewer/

***************************************
* Summary
***************************************
An EDKII implementation of UEFI BIOS requires certain data constructs to be located in different file extensions.  Often times a BIOS developer
cannot remember where a data construct is located because the source tree is huge, and thus must resort to searching the tree.  It is very 
inefficient to search for static data, which is an ideal scenario for a tool to be created to enable a developer to work smarter.  
EdkIIBuildDataViewer fills this void so the developer can spend less time finding data and/or memorizing its location, and spend more time writing
code and debugging issues.


***************************************
* Problem/Opportunity
***************************************
Developers should not spend their brain capacity to memorize a source tree, but rather thinking of ways to innovate, design code, and develop and
debug.Too many times I have witnessed BIOS developers, myself included, use Ctrl-F to search particular package folders, or on many occasions the
entire source tree, to find data constructs related to driver definitions, platform configuration values, and GUIDs.

An EDKII source tree and build logs can be parsed to determine many details: build configuration, how drivers and libraries are declared and what
source files they comprise of, and what source files install a driver and use driver protocols.  There is a wealth of information that can be 
presented to the developer to enable a quicker understanding of the code base and provide clues on how to debug the more complex issues of an 
EDKII BIOS.


***************************************
* Solution
***************************************
The implemented solution parses the build log file for INF files used in the build, and then each INF file is parsed for its [Defines] section for
the driver details, and the [Sources] section for source files used for the driver.  Other sections are parsed for further details on that driver.
Additional information is parsed from a report file if the build was executed with an additional command line switch.  Data is then correlated 
across multiple data structures and displayed to the user, who selects what type of build data they want to view: GUID, PCD, source files, and 
other View choices.  All data is double-clickable to jump to the driver item where that specific data construct is used, edit source files at the
line number where the data construct was found, and all data is searchable.


***************************************
* Release Notes
***************************************
o Revision 1.0.2.5
  + Converted project to Visual Studio 2015.
  + Added tool home page to About dialog box.

o Revision 1.0.1.3
  + Fix bug where paths are broken when Workspace is root of a drive e.g. C:\ instead of a folder e.g. C:\BIOS.  This will happen when you use
    SUBST to assign a drive letter to a path on the hard drive.
  + Change Build Output Directory detection to use "GenFds -o" log output instead of OUTPUT_DIRECTORY from DSC file.

o Revision 1.0.1.2
  + Add command line support.
  + Add command line error checks to output to console or message box.
  + Add AutoIt script (http://www.autoitscript.com/site/autoit) to read a build log and output the source list.

o Revision 1.0.1.1
  + Change writing source files used in build to enable after loading build log instead of build log and build report.

o Revision 1.0.1.0
  + Add support for writing source files used in build to a text file, either in normal or doxygen format.

o Revision 1.0.0.15
  + Initial release.
