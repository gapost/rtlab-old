gpib = dev.newInterface("gpib","GPIB");
gpib.open();
src = dev.newDevice("src",gpib,12,"K6220");
with(src) {
    floating = true
    display = true
    delay = 0.001;
    on();
}
nvm = dev.newDevice("nvm",gpib,7,"K2182");
with(nvm) {
    binaryDataTransfer = true;
    dualChannel = true;
    display = true;
    autoZero = true;
    nplc=1;
    lineSync=1;
    //delay=0.02
    //nplc = 5
    //analogFilter = true
    ch1.multiplier = 1000;
    ch2.multiplier = 1000;
    on();
}

timer = jobs.newTimer("t",333);
timer.newJob("clock","TimeChannel");

jobs.newJob("buff","DataBuffer")
with(jobs.buff)
{
    packetDepth = 1
    maxPackets = 100
    addChannel(timer.clock)
    addChannel(nvm.ch1)
    addChannel(nvm.ch2)
}

with (data) {
    newDataFolder("rt");
    rt.vectorType = "Circular";
    rt.newVector("t");
    rt.newVector("V1");
    rt.newVector("V2");
    rt.t.time=true;
    rt.setCapacity(100);
    rt.setSourceBuffer(jobs.buff);
}

with(figs)
{
    newFigurePane("rt");
    rt.setTitle("Real Time Data");
    rt.fig1.plot(data.rt.t,data.rt.V1)
    rt.fig1.plot(data.rt.t,data.rt.V2)
}


timer.clock.show();
nvm.ch1.show();
nvm.ch2.show();

timer.commit(nvm);
timer.commit(jobs.buff);

jobs.buff.arm();
nvm.arm();
timer.arm()


function startDelta(Io)
{
    with(dev.src)
    {
        disarm();
        off();
        range = Io;
        sweepList = [-Io, Io];
        sweep = true;
        externalTrigger = true;
        on();
        arm();
    }
    nvm.ch1.depth = 3;
    nvm.ch1.averaging = "Delta";
    nvm.ch1.multiplier = 1000/Io;
    nvm.ch2.depth = 3;
    nvm.ch2.averaging = "Delta";
    nvm.ch2.multiplier = 1000/Io*2.425;
    data.rt.clear();
}

function stopDelta()
{
    with(dev.src)
    {
        disarm();
        off();
        sweep = false;
        externalTrigger = false;
        on();
    }
    nvm.ch1.averaging = "None";
    nvm.ch2.averaging = "None";
    data.rt.clear();
}


