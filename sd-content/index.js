async function updateSensors() {
    try {
        const response = await fetch("/sensors");
        const data = await response.json();

        document.getElementById("temperature").textContent =
            data.temperature.toFixed(2);

        document.getElementById("humidity").textContent =
            data.humidity.toFixed(2);

    } catch (error) {
        console.log("Failed to fetch sensors data:", error);
    }
}

setInterval(updateSensors, 1000);
updateSensors();