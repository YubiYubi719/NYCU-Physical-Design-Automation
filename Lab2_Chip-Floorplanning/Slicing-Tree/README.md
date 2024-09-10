## Problem Description
* Inside p2_floorplanning.pdf
* Approaching method: Slicing Tree, following is some suggestion for optimizing
    * Maybe store some necessary information when bottom-up to speed up top-down part (retracing)
    * Don't rebuild whole floorplan if the tree has only a little bit change

## Setup
```makefile
$ chmod 755 testcase/SolutionChecker visualizer
```

## Execution
* Change variable *TESTINPUT* inside Makefile before executing "make run" if you want to test different testcase

```makefile
$ make
$ make run
```

## SolutionChecker
* You can also use the solution checker provided by TAs.
* Remember to change variable *TESTINPUT* inside Makefile before executing "make check"
* Note: The output.txt cannot contain an empty line at the end of the file; otherwise, the solution checker would have some weird problems

```makefile
$ make check
```

## Visualizer
* It's a simple binary that visualize your floorplan, the result figure will be put in *fig* directory

```makefile
$ make figure
```
