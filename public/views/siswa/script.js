let currentUser = null;
let currentPage = 1;
const itemsPerPage = 30;
let groupedData = {};
let allData = [];
let selectedProblems = new Set();

async function loadUserInfo() {
  try {
    const res = await fetch('/api/ext/whoiam');
    if (!res.ok) throw new Error('Unauthorized');

    const data = await res.json(); // data is an array
    currentUser = data[0] || { nama: 'Guest', kelas: '' }; // take the first user or fallback

    document.getElementById('welcome').textContent = 
      `${currentUser.nama} (${currentUser.kelas})`;
  } catch (err) {
    console.error('User fetch error:', err);
    document.getElementById('welcome').textContent = 'Guest';
    currentUser = { nama: 'Guest', kelas: '' };
  }
}
async function loadData() {
  try {
    const response = await fetch('/api/aum/serve');
    if (!response.ok) throw new Error('Failed to load questions.');
    const data = await response.json();
    allData = data;
    window.surveyData = data;

    groupedData = data.reduce((acc, item) => {
      if (!acc[item.nama_bidang_masalah]) acc[item.nama_bidang_masalah] = [];
      acc[item.nama_bidang_masalah].push(item);
      return acc;
    }, {});

    renderPage(currentPage);
    renderPaginationControls();
  } catch (error) {
    console.error('Error loading data:', error);
    document.getElementById('checkboxContainer').textContent = 'Failed to load questions.';
  }
}

function renderPage(page) {
  const container = document.getElementById('checkboxContainer');
  container.innerHTML = '';

  const flatList = Object.values(groupedData).flat();
  const totalPages = Math.ceil(flatList.length / itemsPerPage);
  const start = (page - 1) * itemsPerPage;
  const end = start + itemsPerPage;
  const pageItems = flatList.slice(start, end);

  let index = start + 1;

  pageItems.forEach(problem => {
    const wrapper = document.createElement('div');
    wrapper.className = "flex items-center gap-3";

    const checkbox = document.createElement('input');
    checkbox.type = 'checkbox';
    checkbox.name = 'problems';
    checkbox.value = problem.id;
    checkbox.id = `problem-${problem.id}`;
    checkbox.className = "hidden";

    const label = document.createElement('label');
    label.htmlFor = checkbox.id;
    label.textContent = `${index++}. ${problem.nama_soal_masalah}`;
    label.className = "flex-1 p-3 border border-gray-300 rounded-lg cursor-pointer bg-white text-black transition hover:border-blue-400";

    // Restore checked state
    if (selectedProblems.has(problem.id.toString())) {
      checkbox.checked = true;
      label.classList.add('bg-blue-500', 'text-white', 'border-blue-600');
    }

    checkbox.addEventListener('change', function () {
      if (this.checked) {
        selectedProblems.add(this.value);
        label.classList.add('bg-blue-500', 'text-white', 'border-blue-600');
        label.classList.remove('bg-white', 'text-black', 'border-gray-300');
      } else {
        selectedProblems.delete(this.value);
        label.classList.remove('bg-blue-500', 'text-white', 'border-blue-600');
        label.classList.add('bg-white', 'text-black', 'border-gray-300');
      }
    });

    wrapper.appendChild(checkbox);
    wrapper.appendChild(label);
    container.appendChild(wrapper);
  });

  const submitBtn = document.querySelector('.submit-btn');
  if (submitBtn) {
    submitBtn.classList.toggle('hidden', page !== totalPages);
  }

  window.scrollTo(0, 0);
}


function renderPaginationControls() {
  const flatList = Object.values(groupedData).flat();
  const totalPages = Math.ceil(flatList.length / itemsPerPage);
  const container = document.getElementById('paginationControls');
  container.innerHTML = '';
  if (totalPages <= 1) return;

  const createPageButton = (text, disabled, clickHandler, isActive = false) => {
    const btn = document.createElement('button');
    btn.textContent = text;
    btn.disabled = disabled;
    btn.className = `px-4 py-2 rounded transition ${
      isActive ? 'bg-blue-500 text-white font-bold' : 'bg-white text-blue-800 border border-blue-300 hover:bg-blue-600 hover:text-white hover:scale-105'
    }`;
    btn.addEventListener('click', clickHandler);
    return btn;
  };

  container.appendChild(createPageButton('« Prev', currentPage === 1, () => {
    currentPage--;
    renderPage(currentPage);
    renderPaginationControls();
  }));

  for (let i = 1; i <= totalPages; i++) {
    container.appendChild(createPageButton(i, i === currentPage, () => {
      currentPage = i;
      renderPage(currentPage);
      renderPaginationControls();
    }, i === currentPage));
  }

  container.appendChild(createPageButton('Next »', currentPage === totalPages, () => {
    currentPage++;
    renderPage(currentPage);
    renderPaginationControls();
  }));
}


document.getElementById('surveyForm').addEventListener('submit', function (event) {
  event.preventDefault();

  if (!currentUser) {
    alert('User info not loaded yet!');
    return;
  }

  if (selectedProblems.size === 0) {
    alert('Mohon Mengisi AUM');
    return;
  }

  const combinedArray = Array.from(selectedProblems).map(id => {
    const problem = allData.find(item => item.id.toString() === id);
    return {
      nama: currentUser.nama,
      kelas: currentUser.kelas,
      id: Number(problem.id),
      nama_bidang_masalah: problem.nama_bidang_masalah
    };
  });

 
fetch('/api/aum/submit', {
  method: 'POST',
  headers: { 'Content-Type': 'application/json' },
  body: JSON.stringify(combinedArray)
})
  .then(res => {
    if (res.ok) return res.json();
    // Reject with the response so we can inspect status
    return res.json()
      .catch(() => ({})) // fallback if no JSON body
      .then(errBody => Promise.reject({ status: res.status, body: errBody }));
  })
  .then(json => {
    alert('AUM Berhasil Terkirim');
    window.location.href = '/dashboard/siswa';
  })
  .catch(err => {
    console.error(err);
    // Customize message based on status
    if (err.status === 500) {
      alert('Terjadi kesalahan server. Silakan coba lagi nanti.');
    } else if (err.status === 401) {
      alert('Anda tidak diizinkan mengirim AUM. Silakan login kembali.');
    } else if (err.status === 400) {
      alert('Data tidak valid. Mohon periksa input Anda.');
    } else if (err.status === 409) {
      alert('Anda sudah mengisi AUM.');
    } else {
      alert('AUM Gagal Terkirim. Mohon hubungi guru Anda.');
    }
  });
});
window.onload = async () => {
  await loadUserInfo();
  await loadData();
};

