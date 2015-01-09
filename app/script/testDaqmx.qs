dev.newDAQmxTask("ai");
dev.ai.addAnalogInputChannel("Dev1/ai0")
dev.ai.addAnalogInputChannel("Dev1/ai1")
dev.ai.on()
dev.ai.read()
