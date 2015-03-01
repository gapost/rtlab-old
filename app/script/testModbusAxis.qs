var AxisCtrl = {
    createAxis : function() {
        print("Creating Z-Axis devices ...")
        with(dev) {
                newInterface("modbus","MODBUS-TCP")
                modbus.timeout = 300 // ms
                modbus.host = "100.100.100.5"
                modbus.port = 502
                modbus.open()

                newDevice("z",modbus,0,"Axis")
                z.limitUp = 2000
                z.limitDown = -50000
                z.on()

                print("Current z-axis pos = " + z.pos)
        }
    }
}


