loopPeriod = 1000; // ms



print("Creating Main Aqcuisition Loop ...")
with (jobs) {

    newTimer("t",loopPeriod)
    with (t) {
        newJob("clock","TimeChannel")
        clock.show()
        newJob("test1","TestChannel")
        test1.show()
        newJob("test2","TestChannel")
        test2.show()
        test1.setTestType("Sin");
        test2.setTestType("Pulse");
        newJob("buff","DataBuffer")
        with(buff)
        {
            packetDepth = 1
            maxPackets = 100
            addChannel(t.clock)
            addChannel(t.test1)
            addChannel(t.test2)
        }
    }
}

with (data) {
    newDataFolder("rt");
    rt.vectorType = "Circular";
    rt.newVector("t");
    rt.t.time=true;
    rt.newVector("x");
    rt.newVector("y");
    rt.setSourceBuffer(jobs.t.buff);
    rt.setCapacity(100);
    newDataFolder("buff");
    buff.vectorType = "Open";
    buff.newVector("t");
    buff.t.time=true;
    buff.newVector("x");
    buff.newVector("y");
}

with(figs)
{
    newFigurePane("rt");
    rt.setTitle("Real Time Data");
    rt.fig1.plot(data.rt.t,data.rt.x)
    rt.fig1.plot(data.rt.t,data.rt.y)
    newFigurePane("buff");
    buff.setTitle("Buffered Data");
    buff.fig1.plot(data.buff.t,data.buff.x)
    buff.fig1.plot(data.buff.t,data.buff.y)
}

figs.newWidgetPane("testWidget","form.ui")
with(figs.testWidget)
{
    setTitle("Test Widget")

    ui = widget();

    // 1st tab
    bind(jobs.t.test1,ui.findChild("lcd1"));
    bind(jobs.t.test2,ui.findChild("lcd2"));

}


function start(period) {
    jobs.t.period = period;
    jobs.t.arm()
}

function startRec() {
    data.buff.clear()
    data.buff.setSourceBuffer(jobs.t.buff)
}

function stopRec() {
    data.buff.setSourceBuffer()
}


