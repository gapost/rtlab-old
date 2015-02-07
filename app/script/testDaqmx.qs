function createPressureIndicator(loop,dataBuffer) {
    dev.newDAQmxTask("ai");
    dev.ai.addAnalogInputChannel("Dev1/ai0")
    dev.ai.addAnalogInputChannel("Dev1/ai1")
    with(dev.ai.ch1)
    {
        signalName = "Low Vacuum Gauge"
        unit = "mbar"
        parserExpression = "10^((x-6.143)/1.286)";
        format = "E";
        precision = 1;
    }
    with(dev.ai.ch2)
    {
        signalName = "High Vacuum Gauge"
        unit = "mbar"
        parserExpression = "10^((x-12.66)/1.33)";
        format = "E";
        precision = 1;
    }
    dev.ai.on()
    dev.ai.arm()

    dataBuffer.addChannel(dev.ai.ch1);
    dataBuffer.addChannel(dev.ai.ch2);

    loop.commit(dev.ai);


    with(data.rt)
    {
        newVector(["Plow", "Phigh"])
    }
    with(data.buff)
    {
        newVector(["Plow", "Phigh"])
    }
}

function showPressureChannels(on)
{
    with(dev.ai)
    {
        if (on)
        {
            ch1.show()
            ch2.show()
        }
        else
        {
            ch1.hide()
            ch2.hide()
        }
    }
}


function createIrradiationControl(loop,dataBuffer,loopPeriod)
{
    with(dev)
    {
        newDAQmxTask("beamCounter");
        with (beamCounter) {
            signalName = "Beam Counts"
            unit = "Cnts"
            addCountEdgesChannel("Dev1/ctr0");

            with (ch1)
            {
            newJob("Ib","DataChannel")
            with(Ib)
            {
                signalName = "Average Beam Current"
                unit = "% full scale"
                depth = Math.round(5*1000/loopPeriod) // 5 s averaging
                averaging = "Running"
                multiplier = (1000/loopPeriod)
            }
            }

        }
        beamCounter.on();
        beamCounter.arm();

        newDAQmxTask("beamCap");
        with (beamCap) {
            signalName = "Beam Cap Dig. Out"
            addDigitalOutputChannel("Dev1/port0/line0");
        }
        beamCap.on();


    }

    dataBuffer.addChannel(dev.beamCounter.ch1);
    dataBuffer.addChannel(dev.beamCounter.ch1.Ib);

    loop.commit(dev.beamCounter);


    with(data.rt)
    {
        newVector(["Icnts", "Ib"])
    }
    with(data.buff)
    {
        newVector(["Icnts", "Ib"])
    }

    with(figs)
    {
        newFigurePane("rtBeam");
        rtBeam.setTitle("Real Time Beam Current");
        newFigurePane("buffBeam");
        buffBeam.setTitle("Buffered Beam Current");
    }

    createIrradFigs(figs.rtBeam,  data.rt)
    createIrradFigs(figs.buffBeam,data.buff)

    figs.newWidgetPane("irradCtrl","ui/irradControl.ui")
    with(figs.irradCtrl)
    {
        setTitle("Ion beam Ctrl")

        var ui = widget();

        //bind(dev.beamCap.ch1,"state",ui.findChild("beamOn"))
        // Beam On/Off
        var runButton = ui.findChild("beamOn");
        runButton.clicked.connect(beamOnPressed);


        bind(dev.beamCounter.ch1,ui.findChild("Icnts"));
        bind(dev.beamCounter.ch1.Ib,ui.findChild("Ib"));

    }
}

function createIrradFigs(Figs,Data)
{
    with(Figs)
    {
        with(Data)
        {
            with(fig1)
            {
                plot(t,Ib);
                title="Average Beam Current";
                ylabel="% Full Scale";
                timeScaleX = true;
            }
        }
    }
}

function beamOn()
{
    dev.beamCap.ch1.push(1);
    dev.beamCap.write();
    var ui = figs.irradCtrl.widget();
    var btOn = ui.findChild("beamOn");
    if (!btOn.checked) btOn.toggle(); //checked=true;
}

function beamOff()
{
    dev.beamCap.ch1.push(0);
    dev.beamCap.write();
    var ui = figs.irradCtrl.widget();
    var btOn = ui.findChild("beamOn");
    if (btOn.checked) btOn.toggle(); //checked=false;
}

function beamOnPressed(on)
{
    print("called..." + (on ? "1" : "0"))
    if (on) beamOn();
    else beamOff();
}


function showIrradChannels(on)
{
    with(dev.beamCounter)
    {
        if (on)
        {
            ch1.show()
            ch1.Ib.show()
        }
        else
        {
            ch1.hide()
            ch1.Ib.hide()
        }
    }
}
