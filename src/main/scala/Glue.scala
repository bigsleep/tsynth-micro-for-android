package org.bigsleep.tsynth_micro

class Glue
{
    System.loadLibrary("tsynth")
    
    @native def midiReceive(status : Int, data1 : Int, data2 : Int) : Unit
    @native def setParameterDouble(mod : Int, port : Int, value : Double) : Unit
    @native def setParameterInt(mod : Int, port : Int, value : Int) : Unit
    @native def start() : Unit
    @native def stop() : Unit
}

