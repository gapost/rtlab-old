
t = dev.newInterface("tcp","TCPIP");
d = dev.newDevice("ps",t,0);


with(t) {
    host = "100.100.100.3"
    port = 9221
}
if (t.open() && d.on())
    print(d.query("*IDN?"));
else
    print("Connection failed")

d.off();
t.close();

with(t) {
    host = "100.100.100.4"
    port = 9221
}
if (t.open() && d.on())
    print(d.query("*IDN?"));
else
    print("Connection failed")

d.off();
t.close();

with(t) {
    host = "100.100.100.5"
    port = 9221
}
if (t.open() && d.on())
    print(d.query("*IDN?"));
else
    print("Connection failed")


