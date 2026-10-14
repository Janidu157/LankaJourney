(() => {
  const journeyText = document.getElementById("journeySummary");
  const mapMessage = document.getElementById("mapMessage");
  const stayDetails = document.getElementById("stayDetails");
  const journeyDays = document.getElementById("journeyDays");
  const budgetDetails = document.getElementById("budgetDetails");

  if (window.chrome?.webview) {
    window.chrome.webview.addEventListener("message", event => {
      if (event.data?.type === "journeyResult") displayJourney(event.data.journey);
    });
  } else {
    showEmptyJourney();
  }

  function showEmptyJourney() {
    journeyText.textContent = "No journey is available yet. Enter your preferences to create one.";
    mapMessage.textContent = "";
    const link = document.createElement("a");
    link.className = "btn btn-gold";
    link.href = "planning.html";
    link.textContent = "Plan a journey";
    journeyDays.append(link);
    initializeMap(null);
  }

  function displayJourney(journey) {
    if (!journey) {
      showEmptyJourney();
      return;
    }

    journeyText.textContent = journey.summary || "A journey planned around your requirements.";
    mapMessage.textContent = journey.mapMessage || "";
    initializeMap(journey);

    const stay = journey.accommodation;
    if (stay) {
      const name = document.createElement("h3");
      name.textContent = stay.name;
      stayDetails.append(name);
      const location = getLocation(stay);
      if (location) stayDetails.append(createDetail("Location", location));
      if (stay.type) stayDetails.append(createDetail("Type", stay.type));
      stayDetails.append(createDetail("Price per night", formatMoney(stay.pricePerNight)));
      if (stay.rating > 0) stayDetails.append(createDetail("Rating", `${stay.rating.toFixed(1)} / 5`));
      if (journey.accommodationWithinBudget === false) {
        const warning = document.createElement("p");
        warning.className = "journey-warning";
        warning.textContent = "No listed accommodation fits the nightly budget, so this is the closest available suggestion.";
        stayDetails.append(warning);
      }
    } else {
      stayDetails.textContent = "No accommodation records were found in the database.";
    }

    for (const day of journey.days || []) {
      const card = document.createElement("article");
      card.className = "journey-day-card panel";
      const heading = document.createElement("div");
      heading.className = "journey-day-heading";
      const dayLabel = document.createElement("span");
      dayLabel.className = "journey-day-number";
      dayLabel.textContent = `DAY ${day.day}`;
      heading.append(dayLabel);
      card.append(heading);

      if (day.destination) {
        const destination = document.createElement("h3");
        destination.textContent = day.destination.name;
        card.append(destination);
        const description = [
          day.destination.type,
          day.destination.experience,
          getLocation(day.destination)
        ].filter(Boolean).join(" · ");
        if (description) card.append(createParagraph(description));

        if (Number.isFinite(day.routeDistanceMeters)) {
          const distance = (day.routeDistanceMeters / 1000).toFixed(1);
          const duration = formatDuration(day.routeDuration);
          card.append(createDetail(`Drive from ${day.routeFrom || "the previous mapped stop"}`,
            duration ? `${distance} km · ${duration}` : `${distance} km`));
        } else if (day.routeNotice) {
          card.append(createParagraph(day.routeNotice));
        }
      } else {
        const flexibleDay = document.createElement("h3");
        flexibleDay.textContent = "Flexible day";
        card.append(flexibleDay);
        card.append(createParagraph(
          "No more matching destinations were available in the database; enjoy a relaxed day near your stay."
        ));
      }

      const restaurantHeading = document.createElement("h4");
      restaurantHeading.textContent = "Nearby dining";
      card.append(restaurantHeading);
      if (day.restaurant) {
        const location = getLocation(day.restaurant);
        const rating = day.restaurant.rating > 0 ? ` · ${day.restaurant.rating.toFixed(1)} / 5` : "";
        card.append(createParagraph(`${day.restaurant.name}${location ? ` · ${location}` : ""}${rating}`));
      } else {
        card.append(createParagraph("No nearby restaurant was found in the database."));
      }
      journeyDays.append(card);
    }

    const requirements = JSON.parse(localStorage.getItem("ljPlanning") || "null");
    if (requirements) {
      budgetDetails.append(
        createDetail("Total trip budget", formatMoney(requirements.totalBudget)),
        createDetail("Accommodation budget", formatMoney(requirements.accommodationBudget)),
        createDetail("Food budget", formatMoney(requirements.foodBudget)),
        createDetail("Transport budget", formatMoney(requirements.transportBudget)),
        createDetail("Activity budget", formatMoney(requirements.activityBudget)),
        createDetail("Estimated accommodation total", formatMoney(journey.accommodationEstimate))
      );
      if (requirements.budgetPerDay !== null) {
        budgetDetails.append(createDetail("Your daily budget", formatMoney(requirements.budgetPerDay)));
      }
    }

    if (window.lucide) lucide.createIcons();
  }

  function createDetail(label, value) {
    const row = document.createElement("div");
    row.className = "journey-detail";
    const name = document.createElement("span");
    name.textContent = label;
    const content = document.createElement("strong");
    content.textContent = value;
    row.append(name, content);
    return row;
  }

  function createParagraph(text) {
    const paragraph = document.createElement("p");
    paragraph.className = "journey-muted";
    paragraph.textContent = text;
    return paragraph;
  }

  function getLocation(item) {
    return [item.town, item.district].filter(Boolean).join(", ");
  }

  function formatMoney(amount) {
    return `LKR ${Number(amount || 0).toLocaleString("en-LK", { maximumFractionDigits: 2 })}`;
  }

  function formatDuration(duration) {
    const seconds = Number.parseFloat(duration || "");
    if (!Number.isFinite(seconds)) return "";
    const totalMinutes = Math.round(seconds / 60);
    const hours = Math.floor(totalMinutes / 60);
    const minutes = totalMinutes % 60;
    return hours > 0 ? `${hours} hr ${minutes} min` : `${minutes} min`;
  }

  function initializeMap(journey) {
    if (!window.L) {
      mapMessage.textContent = `${mapMessage.textContent} The interactive map library could not be loaded. Check your internet connection and reload this page.`.trim();
      return;
    }

    const map = L.map("journeyMap", { scrollWheelZoom: false });
    L.tileLayer("https://tile.openstreetmap.de/{z}/{x}/{y}.png", {
      maxZoom: 19,
      attribution: '&copy; <a href="https://www.openstreetmap.org/copyright" target="_blank" rel="noreferrer">OpenStreetMap contributors</a>'
    }).addTo(map);

    const points = [];
    const stay = journey?.accommodation;
    if (hasCoordinates(stay)) {
      points.push({
        latitude: stay.latitude,
        longitude: stay.longitude,
        title: stay.name || "Suggested accommodation",
        description: "Accommodation",
        color: "#b45309"
      });
    }

    for (const day of journey?.days || []) {
      const destination = day.destination;
      if (hasCoordinates(destination)) {
        points.push({
          latitude: destination.latitude,
          longitude: destination.longitude,
          title: destination.name || `Day ${day.day} destination`,
          description: `Day ${day.day}`,
          color: "#1d4ed8"
        });
      }
    }

    const bounds = [];
    for (const point of points) {
      const location = [point.latitude, point.longitude];
      bounds.push(location);
      const marker = L.circleMarker(location, {
        radius: 8,
        color: "#ffffff",
        weight: 2,
        fillColor: point.color,
        fillOpacity: 1
      }).addTo(map);
      const popup = document.createElement("div");
      const title = document.createElement("strong");
      title.textContent = point.title;
      const description = document.createElement("div");
      description.textContent = point.description;
      popup.append(title, description);
      marker.bindPopup(popup);
      marker.bindTooltip(point.description, { direction: "top" });
    }

    const routeCoordinates = journey?.routeCoordinates;
    if (Array.isArray(routeCoordinates) && routeCoordinates.length > 1) {
      const route = L.geoJSON({
        type: "LineString",
        coordinates: routeCoordinates
      }, {
        style: { color: "#c2410c", weight: 5, opacity: 0.85 }
      }).addTo(map);
      const routeBounds = route.getBounds();
      if (routeBounds.isValid()) {
        bounds.push(routeBounds.getSouthWest(), routeBounds.getNorthEast());
      }
    }

    if (bounds.length > 1) {
      map.fitBounds(bounds, { padding: [24, 24], maxZoom: 10 });
    } else if (bounds.length === 1) {
      map.setView(bounds[0], 10);
    } else {
      map.setView([7.8731, 80.7718], 7);
    }
    window.setTimeout(() => map.invalidateSize(), 0);
  }

  function hasCoordinates(item) {
    return item &&
      typeof item.latitude === "number" &&
      typeof item.longitude === "number" &&
      Number.isFinite(item.latitude) &&
      Number.isFinite(item.longitude) &&
      item.latitude >= -90 && item.latitude <= 90 &&
      item.longitude >= -180 && item.longitude <= 180;
  }
})();
