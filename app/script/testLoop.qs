with (jobs) {

    newTimer("t",1000);
    with (t) {
        newJob("clock","TimeChannel");
        clock.show();
    }
}

