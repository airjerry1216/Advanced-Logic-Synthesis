# Advanced Logic Synthesis
### NTHU 10920CS613200 高等邏輯合成


**How to Compile**  
    In this directory, enter the following command:   
    ~~~
    $ make  
    ~~~
    
    It will generate the executable file "als" in this directory.
    If you want to remove it and all .o files, please enter the following command:
    
    $ make clean

--How to Run
    In this directory, enter the following command:   
    Usage: ./<exe> -k 3  <intput.blif file>  <output.blif file>  
    e.g.:
    
    $ ./als -k 3 ../../blif/blif/10aoi_sample01.blif ./output_sample01.blif
