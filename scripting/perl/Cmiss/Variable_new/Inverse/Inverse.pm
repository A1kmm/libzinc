package Cmiss::Variable_new::Inverse;

use 5.006;
use strict;
use warnings;
use Carp;

require Cmiss::Variable_new;
require Exporter;
use AutoLoader;

our @ISA = qw(Cmiss::Variable_new Exporter);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use Cmiss::Variable_new::Inverse ':all';
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
    croak "&Cmiss::Variable_new::Inverse::constant not defined" if $constname eq 'constant';
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

# Named argument
sub new
{
	my ($class, %arg) = @_;
	my ($dependent,$independent,$objref);

	$dependent=$arg{dependent};
	if ($dependent)
	{
		$independent=$arg{independent};
		if ($independent)
		{
			$objref=new_xs($dependent,$independent);
			if ($objref)
			{
				bless $objref,$class;
			}
			else
			{
				croak "Could not create $class";
			}
		}
		else
		{
			croak "Missing independent";
		}
	}
	else
	{
		croak "Missing dependent";
	}
}

sub input_independent
{
	my ($self, %arg) = @_;
	my ($objref);

	$objref=input_independent_xs($self);
	if ($objref)
	{
		bless $objref,'Cmiss::Variable_new_input';
	}
}

sub input_step_tolerance
{
	my ($self, %arg) = @_;
	my ($objref);

	$objref=input_step_tolerance_xs($self);
	if ($objref)
	{
		bless $objref,'Cmiss::Variable_new_input';
	}
}

sub input_value_tolerance
{
	my ($self, %arg) = @_;
	my ($objref);

	$objref=input_value_tolerance_xs($self);
	if ($objref)
	{
		bless $objref,'Cmiss::Variable_new_input';
	}
}

sub input_maximum_iterations
{
	my ($self, %arg) = @_;
	my ($objref);

	$objref=input_maximum_iterations_xs($self);
	if ($objref)
	{
		bless $objref,'Cmiss::Variable_new_input';
	}
}

sub input_dependent_estimate
{
	my ($self, %arg) = @_;
	my ($objref);

	$objref=input_dependent_estimate_xs($self);
	if ($objref)
	{
		bless $objref,'Cmiss::Variable_new_input';
	}
}

# Inherit string conversion
## Overload string and numerical conversion
#use overload '""' => \&string_convert, '0+' => \&numerical_convert, fallback => 1;
#
#sub numerical_convert
#{
#	get_type(shift);
#}
#
#sub string_convert
#{
#	get_type(shift);
#}

require XSLoader;
XSLoader::load('Cmiss::Variable_new::Inverse', $VERSION);

# Preloaded methods go here.

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

Cmiss::Variable_new::Inverse - Perl extension for Cmiss inverse variables

=head1 SYNOPSIS

  use Cmiss::Variable_new::Inverse;

=head1 ABSTRACT

  This should be the abstract for Cmiss::Variable_new::Inverse.
  The abstract is used when making PPD (Perl Package Description) files.
  If you don't want an ABSTRACT you should also edit Makefile.PL to
  remove the ABSTRACT_FROM option.

=head1 DESCRIPTION

Stub documentation for Cmiss::Variable_new::Inverse, created by h2xs. It looks like
the author of the extension was negligent enough to leave the stub
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

