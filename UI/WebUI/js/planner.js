(() => {
  const form = document.getElementById("planningForm");
  if (!form) return;

  const button = form.querySelector('button[type="submit"]');
  const originalButtonMarkup = button?.innerHTML ?? "";
  const pickupMapDialog = document.getElementById("pickupMapDialog");
  const pickupLocationStatus = document.getElementById("pickupLocationStatus");
  const pickupMapSelection = document.getElementById("pickupMapSelection");
  const confirmPickupMap = document.getElementById("confirmPickupMap");
  const airport = {
    name: "Katunayake International Airport",
    latitude: 7.1808,
    longitude: 79.8841
  };
  let pickupMap;
  let pickupMarker;
  let pendingPickup;
  let selectedPickup = airport;

  form.addEventListener("submit", event => {
    event.preventDefault();
    event.stopImmediatePropagation();

    if (!form.reportValidity()) return;

    const destinations = [...form.querySelectorAll('input[name="destinations"]:checked')]
      .map(option => option.value);
    if (destinations.length === 0) {
      showToast("Choose at least one destination or experience.");
      return;
    }

    const days = Number(form.elements.namedItem("days").value);
    const nights = Number(form.elements.namedItem("nights").value);
    if (nights > days) {
      showToast("The number of nights cannot exceed the number of days.");
      return;
    }

    const budgetValue = name => Number(form.elements.namedItem(name).value || 0);
    const startType = form.elements.namedItem("startingPointType").value;
    if (startType === "map" && !selectedPickup.coordinates) {
      showToast("Set your pickup point on the map before exploring your journey.");
      return;
    }
    const startingPoint = startType === "airport"
      ? airport
      : {
        name: form.elements.namedItem("pickupName").value.trim() || "Selected pickup point",
        latitude: selectedPickup.latitude,
        longitude: selectedPickup.longitude
      };
    const plan = {
      accommodationBudget: budgetValue("accommodationBudget"),
      foodBudget: budgetValue("foodBudget"),
      transportBudget: budgetValue("transportBudget"),
      activityBudget: budgetValue("activityBudget"),
      totalBudget: budgetValue("totalBudget"),
      budgetPerDay: form.elements.namedItem("budgetPerDay").value
        ? budgetValue("budgetPerDay")
        : null,
      days,
      nights,
      destinations,
      startingPoint
    };

    localStorage.setItem("ljPlanning", JSON.stringify(plan));

    if (!window.chrome?.webview) {
      showToast("Journey generation is available in the LankaJourney WPF desktop app.");
      return;
    }

    if (button) {
      button.disabled = true;
      button.textContent = "Planning your journey...";
    }
    window.chrome.webview.postMessage(JSON.stringify({ type: "generatePlan", plan }));
  }, { capture: true });

  document.getElementById("openPickupMap").addEventListener("click", openPickupMap);
  form.querySelectorAll('input[name="startingPointType"]').forEach(option => {
    option.addEventListener("change", updatePickupStatus);
  });
  form.elements.namedItem("pickupName").addEventListener("input", updatePickupStatus);
  document.getElementById("closePickupMap").addEventListener("click", () => pickupMapDialog.close());
  document.getElementById("cancelPickupMap").addEventListener("click", () => pickupMapDialog.close());
  document.getElementById("confirmPickupMap").addEventListener("click", () => {
    if (!pendingPickup) return;
    selectedPickup = { ...pendingPickup, coordinates: true };
    form.elements.namedItem("startingPointType").value = "map";
    updatePickupStatus();
    pickupMapDialog.close();
  });
  pickupMapDialog.addEventListener("click", event => {
    if (event.target === pickupMapDialog) pickupMapDialog.close();
  });

  function openPickupMap() {
    if (!window.L) {
      showToast("The map could not be loaded. Check your internet connection and try again.");
      return;
    }
    pendingPickup = selectedPickup.coordinates ? { ...selectedPickup } : null;
    pickupMapSelection.textContent = pendingPickup
      ? formatPickupCoordinates(pendingPickup)
      : "Click the map to select a location.";
    confirmPickupMap.disabled = !pendingPickup;
    pickupMapDialog.showModal();
    if (!pickupMap) {
      pickupMap = L.map("pickupMap", { scrollWheelZoom: false }).setView(
        [selectedPickup.latitude, selectedPickup.longitude], 7
      );
      L.tileLayer("https://tile.openstreetmap.de/{z}/{x}/{y}.png", {
        maxZoom: 19,
        attribution: '&copy; <a href="https://www.openstreetmap.org/copyright" target="_blank" rel="noreferrer">OpenStreetMap contributors</a>'
      }).addTo(pickupMap);
      pickupMap.on("click", event => {
        pendingPickup = {
          latitude: event.latlng.lat,
          longitude: event.latlng.lng
        };
        setPickupMarker(pendingPickup);
        pickupMapSelection.textContent = formatPickupCoordinates(pendingPickup);
        confirmPickupMap.disabled = false;
      });
    }
    window.setTimeout(() => {
      pickupMap.invalidateSize();
      if (pendingPickup) {
        pickupMap.setView([pendingPickup.latitude, pendingPickup.longitude]);
        setPickupMarker(pendingPickup);
      }
    }, 0);
  }

  function setPickupMarker(pickup) {
    const location = [pickup.latitude, pickup.longitude];
    if (pickupMarker) pickupMarker.setLatLng(location);
    else pickupMarker = L.marker(location).addTo(pickupMap);
  }

  function formatPickupCoordinates(pickup) {
    return `Selected location: ${pickup.latitude.toFixed(5)}, ${pickup.longitude.toFixed(5)}`;
  }

  function updatePickupStatus() {
    if (form.elements.namedItem("startingPointType").value === "airport") {
      pickupLocationStatus.textContent = `Starting from ${airport.name}.`;
    } else if (selectedPickup.coordinates) {
      const name = form.elements.namedItem("pickupName").value.trim() || "Selected pickup point";
      pickupLocationStatus.textContent =
        `${name} (${formatPickupCoordinates(selectedPickup).replace("Selected location: ", "")}).`;
    } else {
      pickupLocationStatus.textContent = "Choose a pickup location on the map before planning.";
    }
  }

  if (window.chrome?.webview) {
    window.chrome.webview.addEventListener("message", event => {
      const response = event.data;
      if (response?.type === "journeyError") {
        showToast(response.message || "The journey could not be planned.");
        if (button) {
          button.disabled = false;
          button.innerHTML = originalButtonMarkup;
          if (window.lucide) lucide.createIcons();
        }
        return;
      }

      if (response?.type === "journeyReady") {
        location.href = "journey.html";
      }
    });
  }
})();
