
function createNI6221Tasks() {
    dev.newDAQmxTask("ai");
    with(dev.ai) {
        addAnalogInputChannel("Dev1/ai0","NRSE",0.,10.);
        addAnalogInputChannel("Dev1/ai1","NRSE",0.,10.);
        on();
    }

    dev.newDAQmxTask("ctr");
    with(dev.ctr) {
        addCountEdgesChannel("Dev1/ctr0");
        on();
    }

    dev.newDAQmxTask("dio");
    with(dev.dio) {
        addDigitalOutputChannel("Dev1/port0/line0");
        on();
    }
}

function read() {
    with(dev) {
        ai.read();
        ctr.read();
    }
}

function write(v) {
    dev.dio.ch1.push(v);
    dev.dio.write();
}


/* main()
  */

createNI6221Tasks();

with(dev)
{
    ai.ch1.show();
    ai.ch2.show();
    ctr.ch1.show();
    dio.ch1.show();
}

read();
