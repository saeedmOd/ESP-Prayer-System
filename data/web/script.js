// =====================================
// ESP Prayer System Web Controller
// =====================================


// =====================================
// Clock
// =====================================

function updateClock()
{

    let now = new Date();


    let hours =
        String(now.getHours()).padStart(2,'0');


    let minutes =
        String(now.getMinutes()).padStart(2,'0');


    let seconds =
        String(now.getSeconds()).padStart(2,'0');



    setText(
        "time",
        `${hours}:${minutes}:${seconds}`
    );



    let date =
        now.toLocaleDateString(
            "ar-EG",
            {
                weekday:"long",
                year:"numeric",
                month:"long",
                day:"numeric"
            }
        );



    setText(
        "date",
        date
    );

}



setInterval(
    updateClock,
    1000
);


updateClock();





// =====================================
// Countdown Global Timer
// =====================================

let countdownTimer = null;

let countdownSeconds = 0;





// =====================================
// Load ESP Status
// =====================================

function loadStatus()
{

    fetch("/api/status")


    .then(response => response.json())


    .then(data =>
    {


        // =========================
        // Prayer Times
        // =========================

        setText("fajr", data.fajr);

        setText("dhuhr", data.dhuhr);

        setText("asr", data.asr);

        setText("maghrib", data.maghrib);

        setText("isha", data.isha);



        // =========================
        // Next Prayer
        // =========================

        setText(
            "nextPrayer",
            data.nextPrayer
        );


        setText(
            "nextPrayerTime",
            data.nextPrayerTime
        );



// =========================
// Countdown
// =========================

if (data.countdown !== undefined)
{

    console.log(
        "Countdown from ESP:",
        data.countdown
    );


    let newSeconds =
        Number(data.countdown);



    // أول مرة أو عند تغير القيمة من ESP
    if (
        countdownTimer === null ||
        Math.abs(newSeconds - countdownSeconds) > 10
    )
    {

        countdownSeconds =
            newSeconds;



        if(countdownTimer)
        {
            clearInterval(
                countdownTimer
            );
        }



        updateCountdown();



        countdownTimer =
            setInterval(
                updateCountdown,
                1000
            );

    }

}



        // =========================
        // System Status
        // =========================

        setText(
            "wifi",
            data.wifi ? "متصل" : "غير متصل"
        );


        setText(
            "mqtt",
            data.mqtt ? "متصل" : "غير متصل"
        );


        setText(
            "volume",
            data.volume
        );


    })


    .catch(error =>
    {

        console.log(
            "ESP Offline",
            error
        );

    });


}







// =====================================
// Countdown Update
// =====================================

function updateCountdown()
{

    if(countdownSeconds <= 0)
    {

        setText(
            "countdown",
            "حان الآن وقت الصلاة"
        );

        return;

    }



    let hours =
        Math.floor(
            countdownSeconds / 3600
        );



    let minutes =
        Math.floor(
            (countdownSeconds % 3600) / 60
        );



    let seconds =
        countdownSeconds % 60;



    setText(
        "countdown",
        "بعد " +
        String(hours).padStart(2,'0')
        +
        ":" +
        String(minutes).padStart(2,'0')
        +
        ":" +
        String(seconds).padStart(2,'0')
    );



    countdownSeconds--;

}


// =====================================
// Safe Text Update
// =====================================

function setText(
    id,
    value
)
{

    let element =
        document.getElementById(id);



    if(
        element &&
        value !== undefined &&
        value !== null
    )
    {

        element.innerHTML =
            value;

    }

}







// =====================================
// Test Azan
// =====================================

function testAzan()
{

    fetch(
        "/api/test/azan",
        {
            method:"POST"
        }
    )


    .then(() =>
    {

        alert(
            "تم تشغيل الأذان التجريبي"
        );

    })

    .catch(() =>
    {

        alert(
            "الجهاز غير متصل"
        );

    });

}







// =====================================
// Save Volume
// =====================================

function saveVolume(value)
{

    fetch(
        "/api/settings/volume",
        {

            method:"POST",

            headers:
            {
                "Content-Type":
                "application/json"
            },


            body:
            JSON.stringify(
            {
                volume:value
            })

        }
    );

}







// =====================================
// Refresh Status
// =====================================

setInterval(
    loadStatus,
    5000
);



loadStatus();