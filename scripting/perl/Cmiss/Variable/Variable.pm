package Cmiss::Variable;

use 5.006;
use strict;
use warnings;
use Carp;

require Exporter;
use AutoLoader;

our @ISA = qw(Exporter);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use Cmiss::Variable ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw(
	new
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
	
);

our $VERSION = '0.01';

sub AUTOLOAD {
    # This AUTOLOAD is used to 'autoload' constants from the constant()
    # XS function.

    my $constname;
    our $AUTOLOAD;
    ($constname = $AUTOLOAD) =~ s/.*:://;
    croak "&Cmiss::Variable::constant not defined" if $constname eq 'constant';
    my ($error, $val) = constant($constname);
    if ($error) { croak $error; }
    {
	no strict 'refs';
	# Fixed between 5.005_53 and 5.005_61
#XXX	if ($] >= 5.00561) {
#XXX	    *$AUTOLOAD = sub () { $val };
#XXX	}
#XXX	else {
	    *$AUTOLOAD = sub { $val };
#XXX	}
    }
    goto &$AUTOLOAD;
}

use Cmiss;
Cmiss::require_library('cmgui_computed_variable');

sub new
{
	my ($class, %arg) = @_;
	my ($name,$objref);

	$name=$arg{name};
	if ($name)
	{
		$objref=create($name);
		if ($objref)
		{
			bless $objref,$class;
		}
	}
	else
	{
		croak "Missing name";
	}
}

# Overload string conversion
use overload '""' => \&string_convert, fallback => 1;

sub string_convert
{
	get_type(shift);
}

#???DB.  Replaced with composition computed variable
#sub set_source
#{
#	my ($self, %args) = @_;
#	my ($independent,$source);

#	$independent=$args{independent};
#	if ($independent)
#	{
#		$source=$args{source};
#		#if no source then are clearing the setting
#		if ($source)
#		{
#			set_source_variable($self,$independent,$source);
#		}
#		else
#		{
#			set_source_variable($self,$independent);
#		}
#	}
#	else
#	{
#		croak "Missing independent";
#	}
#}

sub evaluate_derivative
{
	my ($self, %args) = @_;
	my ($independent,$values);

	$independent=$args{independent};
	if ($independent)
	{
		$values=$args{values};
		#if no source then are clearing the setting
		if ($values)
		{
			evaluate_derivative_variable($self,$independent,$values);
		}
		else
		{
			evaluate_derivative_variable($self,$independent);
		}
	}
	else
	{
		croak "Missing independent";
	}
}

require XSLoader;
XSLoader::load('Cmiss::Variable', $VERSION);

# Preloaded methods go here.

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

Cmiss::Variable - Perl extension for Cmiss variables

=head1 SYNOPSIS

  use Cmiss::Variable;

=head1 ABSTRACT

  This should be the abstract for Cmiss::Variable.
  The abstract is used when making PPD (Perl Package Description) files.
  If you don't want an ABSTRACT you should also edit Makefile.PL to
  remove the ABSTRACT_FROM option.

=head1 DESCRIPTION

Stub documentation for Cmiss::Variable, created by h2xs. It looks like the
author of the extension was negligent enough to leave the stub
unedited.


=head2 EXPORT

None by default.



=head1 SEE ALSO

=head1 AUTHOR

David Bullivant, <d.bullivant@auckland.ac.nz>

=head1 COPYRIGHT AND LICENSE

Copyright 2003 by Auckland UniServices

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself. 

=cut

