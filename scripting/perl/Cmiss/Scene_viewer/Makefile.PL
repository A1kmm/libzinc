use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    'NAME'		=> 'Cmiss::Scene_viewer',
    'VERSION_FROM'	=> 'Scene_viewer.pm', # finds $VERSION
    'PREREQ_PM'		=> {}, # e.g., Module::Name => 1.1
    ($] >= 5.005 ?    ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM => 'Scene_viewer.pm', # retrieve abstract from module
       AUTHOR     => '"Shane Blackett <s.blackett@auckland.ac.nz>"') : ()),
    'LIBS'		=> [''], # e.g., '-lm'
    'DEFINE'		=> '', # e.g., '-DHAVE_SOMETHING'
	# Insert -I. if you add *.h files later:
    'INC'		=> "-I$ENV{CMGUI_DEV_SOURCE}", # e.g., '-I/usr/include/other'
	# Turn off "Manifying" because can't create files with names containing :'s in
	#   a FAT32 filesystem
		'INSTALLMAN3DIR' => 'none',
	# Un-comment this if you add C files to link with later:
    # 'OBJECT'		=> '$(O_FILES)', # link all the C files too
);
