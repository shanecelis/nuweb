#!/bin/sh
#
# $RCSfile$-- Test test/00/t0043a.sh
#
#
# Test of Supress indent for fragment expansion
#
work=${TMPDIR:-/tmp}/$$
PAGER=cat
export PAGER
umask 022
here=`pwd`
if test $? -ne 0 ; then exit 2; fi
SHELL=/bin/sh
export SHELL

bin="$here/${1-.}"

pass()
{
	set +x
	cd $here
	rm -rf $work
	exit 0
}
fail()
{
	set +x
	echo "FAILED test of Supress indent for fragment expansion" 1>&2
	cd $here
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT for test of Supress indent for fragment expansion" 1>&2
	cd $here
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

mkdir $work
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi

#
# test Supress indent for fragment expansion
#

cat > test.w <<"EOF"
\documentclass{article}
\begin{document}
@d Fragment one
@{Here is stuff.
   @<Fragment two with @<Multi-line argument@>@>
End of fragment one.
@}

@d Fragment two with @'arg@'
@{Here is frag two stuff, properly indented.
Here is the argument:@s@1
That was the argument.
@}

@d Multi-line argument
@{This stuff
is spread over several lines
and is not indented.
@}

@o test.c -cc
@{Begin
   @<Fragment one@>
end
@}

\end{document}
EOF

cat > test.expected.c <<"EOF"
Begin
   /* Fragment one */
   Here is stuff.
      /* Fragment two with <Multi-line argument> */
      Here is frag two stuff, properly indented.
      Here is the argument:This stuff
is spread over several lines
and is not indented.

      That was the argument.
      
   End of fragment one.
   
end
EOF

# [Add other files here.  Avoid any extra processing such as
# decompression until after demo has run.  If demo fails this script
# can save time by not decompressing. ]

$bin/nuweb test.w
if test $? -ne 0 ; then fail; fi

diff -a --context test.expected.c test.c
if test $? -ne 0 ; then fail; fi

# [Add other sub-tests that might be failed here.  If they need files
# created above to be decompressed, decompress them here ; this saves
# time if demo fails or the text-based sub-test fails.]

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
