#!/usr/bin/perl

# Visits every directory, calls make, and then executes the benchmark
# (Designed for making sure every kernel compiles/runs after modifications)
#
# Written by Tomofumi Yuki, 01/15 2015
#

my $TARGET_DIR = ".";

if ($#ARGV != 0) {
   printf("usage perl run-all.pl target-dir\n");
   exit(1);
}



if ($#ARGV == 0) {
   $TARGET_DIR = $ARGV[0];
}


my @categories = ('linear-algebra/blas',
                  'linear-algebra/kernels',
                  'linear-algebra/solvers',
                  'datamining',
                  'stencils',
                  'medley');


foreach $cat (@categories) {
   my $target = $TARGET_DIR.'/'.$cat;
   opendir DIR, $target or die "directory $target not found.\n";
   while (my $dir = readdir DIR) {
        next if ($dir=~'^\..*');
        next if (!(-d $target.'/'.$dir));

        my $kernel = $dir;
        my $targetDir = $target.'/'.$dir;
        my $command = "cd $targetDir; make clean; make; ./$kernel";
        print($command."\n");
        system($command);
   }

   closedir DIR;
}

