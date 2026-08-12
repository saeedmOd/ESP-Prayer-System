/* =========================================================
   ESP Prayer System
   audio.js

   مسؤول عن:

   - إعدادات الصوت
   - الأذان
   - الإقامة
   - أذكار الصباح
   - أذكار المساء
   - سورة الكهف
   - القرآن
   - التحكم بالتشغيل
   - Volume
   - اختبار الأصوات
   - تحميل / حفظ إعدادات الصوت

   API:
   - /api/settings/audio
   - /api/settings/volume
   - /api/test/azan
   - /api/test/iqama
   - /api/test/morning-adhkar
   - /api/test/evening-adhkar
   - /api/test/kahf
   - /api/test/quran
   - /api/test/folder
   - /api/audio/play
   - /api/audio/pause
   - /api/audio/volume-down
   - /api/audio/stop
   ========================================================= */


/* =========================================================
   Constants
   ========================================================= */

const IQAMA_FOLDER = 2;
const IQAMA_FILE   = 1;


/* =========================================================
   Quran Defaults
   ========================================================= */

const DEFAULT_QURAN_SETTINGS = {

    baqarah: {
        enable: true,
        hour: 0,
        minute: 0,
        volume: 25,
        folder: 2,
        file: 1
    },

    baqarahLast: {
        enable: true,
        hour: 0,
        minute: 0,
        volume: 25,
        folder: 2,
        file: 2
    },

    ayatKursi: {
        enable: true,
        hour: 0,
        minute: 0,
        volume: 25,
        folder: 2,
        file: 3
    },

    maryam: {
        enable: true,
        hour: 0,
        minute: 0,
        volume: 25,
        folder: 2,
        file: 4
    }

};


let quranSettings =
    structuredClone(DEFAULT_QURAN_SETTINGS);


/* =========================================================
   Save State
   ========================================================= */

let audioSaveInProgress = false;


/* =========================================================
   DOM Helpers
   ========================================================= */

function getElement(id) {

    return document.getElementById(id);

}


function showValue(id, value) {

    const element =
        getElement(id);

    if (!element) {
        return;
    }

    element.textContent =
        value ?? "";

}


/* =========================================================
   Input Helpers
   ========================================================= */

function setNumber(id, value) {

    const element =
        getElement(id);

    if (!element) {
        return;
    }

    if (
        value !== undefined &&
        value !== null
    ) {

        element.value =
            value;

    }

}


function setSelect(id, value) {

    const element =
        getElement(id);

    if (!element) {
        return;
    }

    if (
        value === true ||
        value === false
    ) {

        element.value =
            value
                ? "true"
                : "false";

        return;

    }

    element.value =
        String(
            value ?? ""
        );

}


function setCheckbox(id, value) {

    const element =
        getElement(id);

    if (!element) {
        return;
    }

    element.checked =
        value === true;

}


/* =========================================================
   Read Helpers
   ========================================================= */

function readNumber(
    id,
    fallback = 0
) {

    const element =
        getElement(id);

    if (!element) {
        return fallback;
    }

    const value =
        Number(
            element.value
        );

    return Number.isFinite(value)
        ? value
        : fallback;

}


function readBool(
    id,
    fallback = false
) {

    const element =
        getElement(id);

    if (!element) {
        return fallback;
    }


    /*
     * Checkbox
     */

    if (
        element.type === "checkbox"
    ) {

        return element.checked;

    }


    /*
     * Select
     */

    if (
        element.value === "true"
    ) {

        return true;

    }


    if (
        element.value === "false"
    ) {

        return false;

    }


    return fallback;

}


function readCheckbox(
    id,
    fallback = false
) {

    const element =
        getElement(id);

    if (!element) {
        return fallback;
    }

    return Boolean(
        element.checked
    );

}


/* =========================================================
   Preset Helpers
   ========================================================= */

function splitPreset(value) {

    const parts =
        String(value)
            .split(":");

    return {

        folder:
            Number(parts[0]) || 0,

        file:
            Number(parts[1]) || 0

    };

}


function setPresetFromValues(
    type,
    folder,
    file
) {

    const preset =
        getElement(
            type + "Preset"
        );

    if (!preset) {
        return;
    }


    const target =
        Number(folder) +
        ":" +
        Number(file);


    const option =
        Array.from(
            preset.options
        ).find(
            item =>
                item.value === target
        );


    if (option) {

        preset.value =
            target;

    }

}


/* =========================================================
   Preset Actions
   ========================================================= */

function applyPreset(type) {

    const preset =
        getElement(
            type + "Preset"
        );

    if (!preset) {
        return;
    }


    const selected =
        splitPreset(
            preset.value
        );


    console.log(
        "[AUDIO] Preset:",
        type,
        selected.folder,
        selected.file
    );

}


function applyIqamaPreset() {

    applyPreset("iqama");

}


/* =========================================================
   Quran Helpers
   ========================================================= */

function getSelectedQuran() {

    const select =
        getElement(
            "quranType"
        );

    if (!select) {
        return null;
    }

    return select.value || null;

}


function getSelectedQuranFile() {

    const select =
        getElement(
            "quranType"
        );

    if (!select) {
        return null;
    }


    const option =
        select.options[
            select.selectedIndex
        ];

    if (!option) {
        return null;
    }


    return {

        type:
            option.value,

        folder:
            Number(
                option.dataset.folder
            ),

        file:
            Number(
                option.dataset.file
            )

    };

}


/* =========================================================
   Load Selected Quran
   ========================================================= */

function loadSelectedQuran() {

    const type =
        getSelectedQuran();

    if (!type) {
        return;
    }


    const data =
        quranSettings[type];

    if (!data) {
        return;
    }


    setSelect(
        "quranEnable",
        data.enable
    );


    setNumber(
        "quranHour",
        data.hour
    );


    setNumber(
        "quranMinute",
        data.minute
    );


    setNumber(
        "quranVolume",
        data.volume
    );


    showValue(
        "quranVolumeValue",
        data.volume
    );

}


/* =========================================================
   Read Current Quran
   ========================================================= */

function readCurrentQuran() {

    const type =
        getSelectedQuran();

    if (!type) {
        return;
    }


    const previous =
        quranSettings[type] ||
        DEFAULT_QURAN_SETTINGS[type];

    if (!previous) {
        return;
    }


    quranSettings[type] = {

        enable:
            readBool(
                "quranEnable",
                previous.enable
            ),

        hour:
            readNumber(
                "quranHour",
                previous.hour
            ),

        minute:
            readNumber(
                "quranMinute",
                previous.minute
            ),

        volume:
            readNumber(
                "quranVolume",
                previous.volume
            ),

        /*
         * folder/file غير ظاهرين في HTML.
         * نحافظ على القيم القادمة من ESP.
         */

        folder:
            previous.folder,

        file:
            previous.file

    };

}


/* =========================================================
   General Volume
   ========================================================= */

function showVolume(value) {

    showValue(
        "volumeValue",
        value
    );

}


function saveVolume(value) {

    const volume =
        Number(value);

    if (
        !Number.isFinite(volume)
    ) {
        return;
    }


    showValue(
        "volumeValue",
        volume
    );


    /*
     * تحديث أي عنصر Volume موجود في صفحة الصوت.
     */

    setNumber(
        "volume",
        volume
    );


    fetch(
        "/api/settings/volume",
        {

            method: "POST",

            headers: {
                "Content-Type":
                    "application/json"
            },

            body:
                JSON.stringify({
                    volume: volume
                })

        }
    )


    .then(response => {

        if (!response.ok) {

            throw new Error(
                "HTTP " +
                response.status
            );

        }

        return response.text();

    })


    .then(result => {

        console.log(
            "[VOLUME] Saved:",
            result
        );

    })


    .catch(error => {

        console.error(
            "[VOLUME] Save error:",
            error
        );

    });

}


/* =========================================================
   Audio Playback Controls
   ========================================================= */

async function audioControl(
    endpoint,
    successMessage = ""
) {

    try {

        const response =
            await fetch(
                endpoint,
                {
                    method: "POST"
                }
            );


        if (!response.ok) {

            throw new Error(
                "HTTP " +
                response.status
            );

        }


        let result = null;


        /*
         * بعض endpoints قد ترجع JSON
         * وبعضها قد ترجع نصًا فارغًا.
         */

        const contentType =
            response.headers.get(
                "content-type"
            );


        if (
            contentType &&
            contentType.includes(
                "application/json"
            )
        ) {

            result =
                await response.json();

        } else {

            result =
                await response.text();

        }


        console.log(
            "[AUDIO]",
            endpoint,
            result
        );


        if (successMessage) {

            console.log(
                "[AUDIO]",
                successMessage
            );

        }


        return result;

    }


    catch (error) {

        console.error(
            "[AUDIO] Control error:",
            error
        );


        alert(
            "فشل تنفيذ أمر الصوت"
        );


        throw error;

    }

}


/* =========================================================
   Play
   ========================================================= */

function audioPlay() {

    return audioControl(
        "/api/audio/play",
        "تشغيل"
    );

}


/* =========================================================
   Pause
   ========================================================= */

function audioPause() {

    console.log("[UI TEST] audioPause() CLICKED");

    return audioControl(
        "/api/audio/pause",
        "إيقاف مؤقت"
    );
}


/* =========================================================
   Volume Down
   ========================================================= */

function audioVolumeDown() {

    return audioControl(
        "/api/audio/volume-down",
        "خفض الصوت"
    );

}


/* =========================================================
   Stop
   ========================================================= */

function audioStop() {

    return audioControl(
        "/api/audio/stop",
        "إيقاف"
    );

}


/* =========================================================
   Test Endpoint
   ========================================================= */

async function testEndpoint(
    endpoint,
    successMessage
) {

    try {

        const response =
            await fetch(
                endpoint,
                {
                    method: "POST"
                }
            );


        if (!response.ok) {

            throw new Error(
                "HTTP " +
                response.status
            );

        }


        console.log(
            "[AUDIO TEST]",
            endpoint
        );


        alert(
            successMessage
        );


        return true;

    }


    catch (error) {

        console.error(
            "[AUDIO TEST]",
            error
        );


        alert(
            "فشل تشغيل الصوت"
        );


        return false;

    }

}


/* =========================================================
   Test Azan
   ========================================================= */

function testAzan() {

    return testEndpoint(
        "/api/test/azan",
        "تم تشغيل الأذان"
    );

}


/* =========================================================
   Test Iqama
   ========================================================= */

function testIqama() {

    return testEndpoint(
        "/api/test/iqama",
        "تم تشغيل الإقامة"
    );

}


/* =========================================================
   Test Morning Adhkar
   ========================================================= */

function testMorningAdhkar() {

    return testEndpoint(
        "/api/test/morning-adhkar",
        "تم تشغيل أذكار الصباح"
    );

}


/* =========================================================
   Test Evening Adhkar
   ========================================================= */

function testEveningAdhkar() {

    return testEndpoint(
        "/api/test/evening-adhkar",
        "تم تشغيل أذكار المساء"
    );

}


/* =========================================================
   Test Kahf
   ========================================================= */

function testKahf() {

    return testEndpoint(
        "/api/test/kahf",
        "تم تشغيل سورة الكهف"
    );

}


/* =========================================================
   Test Quran
   ========================================================= */

async function testQuran() {

    /*
     * حفظ قيم القرآن الحالية أولًا.
     */

    readCurrentQuran();


    const selected =
        getSelectedQuranFile();


    if (!selected) {

        alert(
            "لم يتم اختيار محتوى القرآن"
        );

        return;

    }


    const data =
        quranSettings[
            selected.type
        ];


    if (!data) {

        alert(
            "إعدادات المحتوى غير موجودة"
        );

        return;

    }


    console.log(
        "[QURAN TEST]",
        {
            type:
                selected.type,

            folder:
                selected.folder,

            file:
                selected.file,

            volume:
                data.volume
        }
    );


    try {

        const response =
            await fetch(
                "/api/test/quran",
                {

                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify({

                            type:
                                selected.type,

                            folder:
                                selected.folder,

                            file:
                                selected.file,

                            volume:
                                data.volume

                        })

                }
            );


        if (!response.ok) {

            throw new Error(
                "HTTP " +
                response.status
            );

        }


        const result =
            await response.text();


        console.log(
            "[QURAN TEST] Response:",
            result
        );


        alert(
            "تم تشغيل المحتوى المختار"
        );


        return true;

    }


    catch (error) {

        console.error(
            "[QURAN TEST]",
            error
        );


        alert(
            "فشل تشغيل المحتوى المختار"
        );


        return false;

    }

}


/* =========================================================
   Play Folder
   ========================================================= */

async function playFolder() {

    const folder =
        readNumber(
            "folder",
            1
        );


    const volume =
        readNumber(
            "folderVolume",
            20
        );


    try {

        const response =
            await fetch(
                "/api/test/folder",
                {

                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify({

                            folder:
                                folder,

                            volume:
                                volume

                        })

                }
            );


        if (!response.ok) {

            throw new Error(
                "HTTP " +
                response.status
            );

        }


        alert(
            "تم تشغيل المجلد"
        );


        return true;

    }


    catch (error) {

        console.error(
            "[FOLDER TEST]",
            error
        );


        alert(
            "فشل تشغيل المجلد"
        );


        return false;

    }

}


/* =========================================================
   Build Audio Payload
   ========================================================= */

function buildAudioPayload() {

    /*
     * حفظ إعداد القرآن الحالي.
     */

    readCurrentQuran();


    /* =====================================================
       Azan
       ===================================================== */

    let azanFolder = 1;
    let azanFile   = 1;


    const azanPreset =
        getElement(
            "azanPreset"
        );


    if (azanPreset) {

        const selected =
            splitPreset(
                azanPreset.value
            );


        azanFolder =
            selected.folder;

        azanFile =
            selected.file;

    }


    /* =====================================================
       Iqama
       ===================================================== */

    let iqamaFolder =
        IQAMA_FOLDER;

    let iqamaFile =
        IQAMA_FILE;


    const iqamaPreset =
        getElement(
            "iqamaPreset"
        );


    if (iqamaPreset) {

        const selected =
            splitPreset(
                iqamaPreset.value
            );


        iqamaFolder =
            selected.folder;

        iqamaFile =
            selected.file;

    }


    /* =====================================================
       Payload
       ===================================================== */

    return {

        /* General */

        volume:
            readNumber(
                "volume",
                15
            ),


        /* Azan */

        azanEnable:
            readBool(
                "azanEnable",
                true
            ),

        azanFolder:
            azanFolder,

        azanFile:
            azanFile,


        /* Iqama */

        iqamaEnable:
            readBool(
                "iqamaEnable",
                true
            ),

        iqamaFolder:
            iqamaFolder,

        iqamaFile:
            iqamaFile,

        iqamaVolume:
            readNumber(
                "iqamaVolume",
                12
            ),

        iqamaDelay:
            readNumber(
                "iqamaDelay",
                10
            ),

        iqamaFajr:
            readCheckbox(
                "iqamaFajr",
                true
            ),

        iqamaDhuhr:
            readCheckbox(
                "iqamaDhuhr",
                true
            ),

        iqamaAsr:
            readCheckbox(
                "iqamaAsr",
                true
            ),

        iqamaMaghrib:
            readCheckbox(
                "iqamaMaghrib",
                true
            ),

        iqamaIsha:
            readCheckbox(
                "iqamaIsha",
                true
            ),


        /* Morning Adhkar */

        morningAdhkarEnable:
            readBool(
                "morningAdhkarEnable",
                false
            ),

        morningAdhkarHour:
            readNumber(
                "morningAdhkarHour",
                6
            ),

        morningAdhkarMinute:
            readNumber(
                "morningAdhkarMinute",
                0
            ),

        morningAdhkarVolume:
            readNumber(
                "morningAdhkarVolume",
                25
            ),


        /* Evening Adhkar */

        eveningAdhkarEnable:
            readBool(
                "eveningAdhkarEnable",
                false
            ),

        eveningAdhkarHour:
            readNumber(
                "eveningAdhkarHour",
                18
            ),

        eveningAdhkarMinute:
            readNumber(
                "eveningAdhkarMinute",
                0
            ),

        eveningAdhkarVolume:
            readNumber(
                "eveningAdhkarVolume",
                25
            ),


        /* Kahf */

        kahfEnable:
            readBool(
                "kahfEnable",
                false
            ),

        kahfHour:
            readNumber(
                "kahfHour",
                9
            ),

        kahfMinute:
            readNumber(
                "kahfMinute",
                0
            ),

        kahfVolume:
            readNumber(
                "kahfVolume",
                25
            ),


        /* Quran */

        quran:
            quranSettings

    };

}


/* =========================================================
   Load Audio Settings
   ========================================================= */

async function loadAudioSettings() {

    console.log(
        "[AUDIO] Loading settings..."
    );


    try {

        const response =
            await fetch(
                "/api/settings/audio",
                {
                    method: "GET",
                    cache: "no-store"
                }
            );


        if (!response.ok) {

            throw new Error(
                "HTTP " +
                response.status
            );

        }


        const data =
            await response.json();


        console.log(
            "[AUDIO] Settings loaded:",
            data
        );


        /* =================================================
           General Volume
           ================================================= */

        const volume =
            data.volume ?? 15;


        setNumber(
            "volume",
            volume
        );


        showVolume(
            volume
        );


        /* =================================================
           Azan
           ================================================= */

        setSelect(
            "azanEnable",
            data.azanEnable
        );


        if (
            data.azanFolder !== undefined &&
            data.azanFile !== undefined
        ) {

            setPresetFromValues(
                "azan",
                data.azanFolder,
                data.azanFile
            );

        }


        /* =================================================
           Iqama
           ================================================= */

        setSelect(
            "iqamaEnable",
            data.iqamaEnable
        );


        setNumber(
            "iqamaVolume",
            data.iqamaVolume ?? 12
        );


        showValue(
            "iqamaVolumeValue",
            data.iqamaVolume ?? 12
        );


        setNumber(
            "iqamaDelay",
            data.iqamaDelay ?? 10
        );


        setCheckbox(
            "iqamaFajr",
            data.iqamaFajr
        );


        setCheckbox(
            "iqamaDhuhr",
            data.iqamaDhuhr
        );


        setCheckbox(
            "iqamaAsr",
            data.iqamaAsr
        );


        setCheckbox(
            "iqamaMaghrib",
            data.iqamaMaghrib
        );


        setCheckbox(
            "iqamaIsha",
            data.iqamaIsha
        );


        if (
            data.iqamaFolder !== undefined &&
            data.iqamaFile !== undefined
        ) {

            setPresetFromValues(
                "iqama",
                data.iqamaFolder,
                data.iqamaFile
            );

        }


        /* =================================================
           Morning Adhkar
           ================================================= */

        setSelect(
            "morningAdhkarEnable",
            data.morningAdhkarEnable
        );


        setNumber(
            "morningAdhkarHour",
            data.morningAdhkarHour ?? 6
        );


        setNumber(
            "morningAdhkarMinute",
            data.morningAdhkarMinute ?? 0
        );


        setNumber(
            "morningAdhkarVolume",
            data.morningAdhkarVolume ?? 25
        );


        showValue(
            "morningAdhkarVolumeValue",
            data.morningAdhkarVolume ?? 25
        );


        /* =================================================
           Evening Adhkar
           ================================================= */

        setSelect(
            "eveningAdhkarEnable",
            data.eveningAdhkarEnable
        );


        setNumber(
            "eveningAdhkarHour",
            data.eveningAdhkarHour ?? 18
        );


        setNumber(
            "eveningAdhkarMinute",
            data.eveningAdhkarMinute ?? 0
        );


        setNumber(
            "eveningAdhkarVolume",
            data.eveningAdhkarVolume ?? 25
        );


        showValue(
            "eveningAdhkarVolumeValue",
            data.eveningAdhkarVolume ?? 25
        );


        /* =================================================
           Kahf
           ================================================= */

        setSelect(
            "kahfEnable",
            data.kahfEnable
        );


        setNumber(
            "kahfHour",
            data.kahfHour ?? 9
        );


        setNumber(
            "kahfMinute",
            data.kahfMinute ?? 0
        );


        setNumber(
            "kahfVolume",
            data.kahfVolume ?? 25
        );


        showValue(
            "kahfVolumeValue",
            data.kahfVolume ?? 25
        );


        /* =================================================
           Quran
           ================================================= */

        if (
            data.quran &&
            typeof data.quran === "object"
        ) {

            Object.keys(
                quranSettings
            ).forEach(
                type => {

                    if (
                        data.quran[type] &&
                        typeof data.quran[type] === "object"
                    ) {

                        quranSettings[type] = {

                            ...quranSettings[type],

                            ...data.quran[type]

                        };

                    }

                }
            );

        }


        loadSelectedQuran();


        /* =================================================
           Status
           ================================================= */

        const status =
            getElement(
                "status"
            );


        if (status) {

            status.textContent =
                "متصل";

        }


        console.log(
            "[AUDIO] Settings loaded successfully"
        );

    }


    catch (error) {

        console.error(
            "[AUDIO] Load error:",
            error
        );


        const status =
            getElement(
                "status"
            );


        if (status) {

            status.textContent =
                "خطأ اتصال";

        }

    }

}


/* =========================================================
   Save Audio Settings
   ========================================================= */

async function saveAudioSettings() {

    /*
     * منع الضغط المتكرر.
     */

    if (audioSaveInProgress) {

        console.warn(
            "[AUDIO] Save already in progress"
        );

        return;

    }


    audioSaveInProgress =
        true;


    const status =
        getElement(
            "status"
        );


    try {

        console.log(
            "[AUDIO] Preparing settings..."
        );


        const data =
            buildAudioPayload();


        console.log(
            "[AUDIO] Saving:",
            data
        );


        if (status) {

            status.textContent =
                "جارٍ الحفظ...";

        }


        const response =
            await fetch(
                "/api/settings/audio",
                {

                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(data)

                }
            );


        if (!response.ok) {

            throw new Error(
                "HTTP " +
                response.status
            );

        }


        const result =
            await response.text();


        console.log(
            "[AUDIO] Save response:",
            result
        );


        if (status) {

            status.textContent =
                "تم الحفظ";

        }


        alert(
            "تم حفظ إعدادات الصوت بنجاح"
        );


        return true;

    }


    catch (error) {

        console.error(
            "[AUDIO] Save error:",
            error
        );


        if (status) {

            status.textContent =
                "خطأ في الحفظ";

        }


        alert(
            "فشل حفظ إعدادات الصوت"
        );


        return false;

    }


    finally {

        audioSaveInProgress =
            false;

    }

}


/* =========================================================
   Event Bindings
   ========================================================= */

function initAudioEvents() {

    /* =====================================================
       Quran Type
       ===================================================== */

    const quranType =
        getElement(
            "quranType"
        );


    if (quranType) {

        quranType.addEventListener(
            "change",
            () => {

                /*
                 * حفظ العنصر السابق.
                 */

                readCurrentQuran();


                /*
                 * تحميل العنصر الجديد.
                 */

                loadSelectedQuran();

            }
        );

    }


    /* =====================================================
       Volume Sliders
       ===================================================== */

    const volumeBindings = [

        {
            input: "volume",
            output: "volumeValue"
        },

        {
            input: "iqamaVolume",
            output: "iqamaVolumeValue"
        },

        {
            input: "morningAdhkarVolume",
            output: "morningAdhkarVolumeValue"
        },

        {
            input: "eveningAdhkarVolume",
            output: "eveningAdhkarVolumeValue"
        },

        {
            input: "kahfVolume",
            output: "kahfVolumeValue"
        },

        {
            input: "quranVolume",
            output: "quranVolumeValue"
        }

    ];


    volumeBindings.forEach(
        binding => {

            const input =
                getElement(
                    binding.input
                );


            if (!input) {
                return;
            }


            input.addEventListener(
                "input",
                () => {

                    showValue(
                        binding.output,
                        input.value
                    );

                }
            );

        }
    );

}


/* =========================================================
   Startup
   ========================================================= */

window.addEventListener(
    "DOMContentLoaded",
    async () => {

        initAudioEvents();

        await loadAudioSettings();

    }
);