if (!defined $path)
{
  $path = ".";
}

# testing Cmiss::Variable_new::Scalar

use Cmiss::Variable_new;
use Cmiss::Variable_new::Scalar;
use Cmiss::Variable_new::Derivative_matrix;
use Cmiss::Variable_new_input;
use Cmiss::Variable_new_input::Composite;

# check creating scalar variable
$var_1=new Cmiss::Variable_new::Scalar(value=>5);
print "$var_1\n";
# check evaluating with no arguments
$var_2=$var_1->evaluate();
print "$var_2 $var_1\n";
# make another variable to be used as an input when evaluating
$var_3=new Cmiss::Variable_new::Scalar(value=>3);
print "$var_3 $var_1\n";
# check evaluating with arguments with the arguments being for the variable
#   being evaluated.  Check that variable being evaluated isn't changed
$var_4=$var_1->evaluate($var_1->input_value(),$var_3);
print "$var_4 $var_1\n";
# check evaluating with arguments with the arguments not being for the variable
#   being evaluated.  Also checks that the variable which was created by
#   evaluating has the correct type (Scalar, input_value is only defined for
#   Scalar)
$var_5=$var_1->evaluate($var_2->input_value(),$var_3);
print "$var_5 $var_1\n";
# check evaluating first derivative
$var_6=$var_1->evaluate_derivative(independent=>[$var_1->input_value()]);
print "$var_6\n";
$var_7=$var_1->evaluate_derivative(independent=>[$var_2->input_value()]);
print "$var_7\n";
# check evaluating second derivative
$input_1=$var_1->input_value();
$var_8=$var_1->evaluate_derivative(independent=>[$input_1,$input_1]);
print "$var_8\n";
# check creating composite input
$input_2=new Cmiss::Variable_new_input::Composite($input_1,$var_2->input_value());
# check evaluating first derivative wrt composite
$var_9=$var_1->evaluate_derivative(independent=>[$input_2]);
print "$var_9\n";
$var_10=$var_1->evaluate_derivative(independent=>[new Cmiss::Variable_new_input::Composite($var_2->input_value(),$input_1)]);
print "$var_10\n";
# check getting and setting input values
$var_11=$var_1->get_input_value($input_1);
print "$var_11\n";
$return_code=$var_1->set_input_value($input_1,$var_3);
print "$return_code $var_1\n";
$var_12=$var_1->get_input_value($var_2->input_value());
if (defined($var_12))
{
	print "$var_12\n";
} else
{
	print "undefined\n";
}
$return_code=$var_1->set_input_value($var_2->input_value(),$var_2);
print "$return_code $var_1\n";
