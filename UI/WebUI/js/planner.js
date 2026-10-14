(() => {
  const form = document.getElementById("planningForm");
  if (!form) return;

  const button = form.querySelector('button[type="submit"]');
  const originalButtonMarkup = button?.innerHTML ?? "";

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
      destinations
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
