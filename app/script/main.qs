exec("testDaqmx.qs")

loopPeriod = 333; // ms
measDelay = 3; // 3 x 333 = 1 s


print("Creating Main Aqcuisition Loop ...")
with (jobs) {

    newTimer("t",loopPeriod)
    with (t) {
        newJob("clock","TimeChannel")
        newDelayLoop("cryoLoop",1)
        newDelayLoop("deltaLoop",1)
        //newDelayLoop("sampleLoop",1)
        newDelayLoop("slow",3)
        newDelayLoop("measLoop",measDelay)
    }

    newJob("buff","DataBuffer")
    with(buff)
    {
        packetDepth = 1
        maxPackets = 100
        addChannel(t.clock)
    }

    with (t.measLoop) {
        commit(buff);
    }
    buff.arm();
}

with (data) {
    newDataFolder("rt");
    rt.vectorType = "Circular";
    rt.newVector("t");
    rt.t.time=true;
    rt.setSourceBuffer(jobs.buff);
    newDataFolder("buff");
    buff.vectorType = "Open";
    buff.newVector("t");
    buff.t.time=true;
}


// creating all systems
createIrradiationControl(jobs.t.measLoop,jobs.buff,loopPeriod*measDelay)
createPressureIndicator(jobs.t.measLoop,jobs.buff)

jobs.t.clock.show();
showIrradChannels(1);
showPressureChannels(1);
jobs.t.arm()
