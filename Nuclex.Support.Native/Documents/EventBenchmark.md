Benchmark for Nuclex::Support::Events
=====================================

Several popular signal/slot libraries were run through between 20480000 and 512000000
repetitions of an operation and the total time was measured.

This was done on different CPUs, platforms and compilers: 
  * Intel Xeon E5-2680 w/Visual Studio 2017
  * Intel Xeon E5-2680 w/GCC 8.3
  * Intel Xeon E5-2680 w/Clang 8.0
  * Intel i5 w/Visual Studio 2017
  * Intel Celeron N4100 w/Visual Studio 2017
  * Intel Celeron N3160 w/GCC 8.3
  * Intel Celeron N3160 w/Clang 8.0
  * AMD Opteron 3365 w/GCC 8.3
  * AMD Opteron 3365 w/Clang 8.0

I'd love to have some more AMD representation, like a Ryzen system, but I didn't have
access to one when doing the benchmarks.

I then calculated the average number of CPU cycles one operation would take using
the following formula:

    cycles_per_action = (cpu_speed_ghz x 1,000,000,000) / (total_time / number_of_repeats)

This is a somewhat logical number that fits on a scale and that anyone can translate into
expected performance numbers depending on target hardware and parallelism.


CPU Cycles per Action (lower is better)
---------------------------------------

Graphical version at http://blog.nuclex-games.com/2019/10/nuclex-signal-slot-benchmarks


                     + ============================================================ // ==  1219.085
                     - ============================================================ // ==  1176.319
                     c ============================================================ // ==  1490.319
    Boost.Signals2   C ============================================================ // ==  1740.037
                     d ==========================================  622.899
                     D ================================================================  944.905
                     i ============================  416.19
                     I =================  255.902
    
    
                     + =  1.919
                     - =  2.876
                     c =================================================================  961.125
    libsigc++        C ============================================================ // ==  1453.901
                     d ==============================================  684.488
                     D ============================================================ // ==  1128.927
                     i =========  137.687
                     I ======  96.000
    
    
                     + ======  88.650
                     - ======  87.616
                     c ============================  416.138
    LSignal          C ============================================================ // ==  1399.726
                     d ================  239.528
                     D ==========================================  626.398
                     i ===  46.843
                     I ====  58.714
    
    
                     + ==  35.906
                     - ==  30.009
                     c ==========  150.337
    Nano Signal '11  C ====================  300.744
                     d ===========  164.583
                     D ====================  297.294
                     i ==  33.763
                     I ====  63.472
    
    
                     + =  2.068
                     - =  9.472
                     c =============  191.820
    Nano Signal '17  C ===================  279.241
                     d =============  202.253
                     D =======================================================  818.927
                     i =  17.362
                     I =  37.486
    
    
                     + =  4.597
                     - =  4.385
                     c =  20.532
    Nuclex           C ==  39.282
                     d =  24.629
                     D ==  34.009
                     i =  20.542
                     I =  23.517
    
    
                     + =  25.416
                     - ==  33.991
                     c ============================  410.259
    Sigs             C ========================================  601.962
                     d ==============================  448.310
                     D ============================================================ // ==  1543.539
                     i ==  34.104
                     I ====  52.597
    
    
                     +++ = Construct | --- = Destruct | ccc = Connect 2 | CCC = Connect 50
                     ddd = Detach 2 | DDD = Detach 50 | iii = Invoke 2 | III = Invoke 50
