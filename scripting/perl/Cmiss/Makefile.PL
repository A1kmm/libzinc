use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
#
#???DB.  Notes:
# setenv CMGUI_DEV_SOURCE /dosD/cmiss/cmgui/source
# To install locally, use
#   comment out the line 'DIR'   => which limits the directories that get released
#   perl Makefile.PL PREFIX=${CMISS_ROOT}/cmiss_perl
#     OR to build DEBUG
#   perl Makefile.PL PREFIX=${CMISS_ROOT}/cmiss_perl OPTIMIZE="-g -W -Wall -Wno-parentheses -Wno-switch -Wno-format -Wno-unused -Werror"
#   make
#   make install
# To rebuild cmgui libraries
#   make -f cmgui.make TARGET=so_lib
#   cp bin/i686-linux/libcmgui-debug.so ${HOME}/lib/libcmgui.so
#   cp bin/i686-linux/libcmgui_computed_variable-debug.so ${HOME}/lib/libcmgui_computed_variable.so
#   cp bin/i686-linux/libcmgui_computed_variable_finite_element-debug.so ${HOME}/lib/libcmgui_computed_variable_finite_element.so
#   cp bin/i686-linux/libcmgui_general-debug.so ${HOME}/lib/libcmgui_general.so
#   cp bin/i686-linux/libcmgui_finite_element-debug.so ${HOME}/lib/libcmgui_finite_element.so
# To test without installing
#   setenv LD_LIBRARY_PATH ${HOME}/lib:${CMISS_ROOT}/mesa/lib/i686-linux
#   make test

#Recursively look up from here for typemap files.
my $typemap_file = "typemap";
my @typemap_array = ();
while ( -f $typemap_file )
  {
	 #We want the typemap closest to us to have the highest precedence which
	 #is last in the list.
	 unshift @typemap_array, $typemap_file;
	 $typemap_file = "../$typemap_file";
  }

use Config;

WriteMakefile(
    'NAME'    => 'Cmiss',
  #          
  #Only install a subset of "released modules" at the moment
	 'DIR'   =>  [ 'Graphics_window', 'Scene_viewer', 'Perl_cmiss', 'Cmiss_context',
						'Example_path', 'Element', 'Node', 'Region', 'Field'  ],

  #Add the perl version into the site library
    'INSTALLSITELIB'   =>  "\$(PREFIX)/lib/$Config{version}",
	 'INSTALLSITEARCH'   =>  "\$(PREFIX)/lib/$Config{version}/$Config{archname}",

    'VERSION_FROM'  => 'Cmiss.pm', # finds $VERSION
    'PREREQ_PM'    => {}, # e.g., Module::Name => 1.1
    ($] >= 5.005 ?    ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM => 'Cmiss.pm', # retrieve abstract from module
       AUTHOR     => '"Shane Blackett <shane@blackett.co.nz>"') : ()),
    'LIBS'    => [''], # e.g., '-lm'
    # 'DEFINE'    => '-DFULL_NAMES', # e.g., '-DHAVE_SOMETHING'
  # Insert -I. if you add *.h files later:
    'INC'    => "-I$ENV{CMGUI_DEV_SOURCE}", # e.g., '-I/usr/include/other'
	# Turn off "Manifying" because can't create files with names containing :'s in
	#   a FAT32 filesystem
		'INSTALLMAN3DIR' => 'none',
  # Un-comment this if you add C files to link with later:
    # 'OBJECT'    => '$(O_FILES)', # link all the C files too
  # Include the typemap files of any directories above here
    'TYPEMAPS'    =>  \@typemap_array,
);
