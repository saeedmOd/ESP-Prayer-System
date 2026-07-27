// =====================================
// ESP Prayer System Web Controller
// =====================================


// تحديث الساعة محلياً (مؤقت)
// لاحقاً سيتم استبداله بوقت ESP الحقيقي
function updateClock()
{

    let now = new Date();


    let hours = String(now.getHours()).padStart(2,'0');

    let minutes = String(now.getMinutes()).padStart(2,'0');

    let seconds = String(now.getSeconds()).padStart(2,'0');



    document.getElementById("time").innerHTML =
        `${hours}:${minutes}:${seconds}`;



    let date = now.toLocaleDateString(
        "ar-EG",
        {
            weekday:"long",
            year:"numeric",
            month:"long",
            day:"numeric"
        }
    );


    document.getElementById("date").innerHTML =
        date;

}




setInterval(
    updateClock,
    1000
);

updateClock();





// =====================================
// Load ESP Status
// =====================================

function loadStatus()
{

    fetch("/api/status")


    .then(response => response.json())


    .then(data =>
    {


        if(data.wifi)
        {
            document.getElementById("wifi").innerHTML =
                data.wifi;
        }


        if(data.mqtt)
        {
            document.getElementById("mqtt").innerHTML =
                data.mqtt;
        }


        if(data.volume)
        {
            document.getElementById("volume").innerHTML =
                data.volume;
        }


        if(data.nextPrayer)
        {
            document.getElementById("nextPrayer").innerHTML =
                data.nextPrayer;
        }


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
// Test Azan
// =====================================

function testAzan()
{


    fetch("/api/test/azan",
    {

        method:"POST"

    })


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


    fetch("/api/settings/volume",
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

    });


}





// =====================================
// Load Every 5 Seconds
// =====================================

setInterval(
    loadStatus,
    5000
);


loadStatus();