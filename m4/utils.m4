###############################################################################
# Filter out unwanted values (space separated) from a list of allowed values.
#
# $1: result variable name
# $2: list of values to check
# $3: list of allowed values
AC_DEFUN([FILTER_VALUES],
[
  check=`echo $2 | tr ' ' '\n'`
  allowed=`echo $3 | tr ' ' '\n'`
  if test -z "$allowed"; then
    $1="$2"
  else
    result=`grep -Fvx -- "$allowed" <<< "$check" | grep -v '^$'`
    $1=${result//$'\n'/ }
  fi
])
