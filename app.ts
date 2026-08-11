const button = document.querySelector<HTMLButtonElement>("#request");
const result = document.querySelector<HTMLElement>("#result");

if (!button || !result) throw new Error("Required interface elements are missing");

button.addEventListener("click", async () => {
  button.disabled = true;
  result.textContent = "Loading...";

  try {
    const response = await fetch("/api/hello");
    const data = await response.json();
    result.textContent = JSON.stringify(data, null, 2);
  } catch (error) {
    result.textContent = error instanceof Error ? error.message : "Request failed";
  } finally {
    button.disabled = false;
  }
});
