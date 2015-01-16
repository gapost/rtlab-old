function createNI6221Tasks() {
    dev.newDAQmxTask("ai");
    with(dev.ai) {
        addAnalogInputChannel("Dev1/ai0","NRSE",0.,10.);
        addAnalogInputChannel("Dev1/ai1","NRSE",0.,10.);
        on();
    }

}

function read() {
    with(dev) {
        ai.read();
    }
}




/* main()
  */

createNI6221Tasks();

with(dev)
{
    ai.ch1.show();
    ai.ch2.show();

}

read();
