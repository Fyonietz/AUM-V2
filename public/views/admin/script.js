// ============================================
// ADMIN DASHBOARD - MAIN JAVASCRIPT FILE
// ============================================

// ==========================================
// SIDEBAR & NAVIGATION FUNCTIONS
// ==========================================

function toggleSidebar() {
    const sidebar = document.getElementById('sidebar');
    const overlay = document.getElementById('overlay');
    sidebar.classList.toggle('-translate-x-full');
    overlay.classList.toggle('hidden');
}

// ==========================================
// KELAS MANAGEMENT (Create / Delete)
// ==========================================

async function createKelas() {
    const name = (document.getElementById('newKelasName')?.value || '').trim();
    const msg = document.getElementById('kelasMessage');
    if (!name) {
        if (msg) { msg.textContent = 'Masukkan nama kelas.'; msg.className = 'text-red-600'; }
        return;
    }

    try {
        const res = await fetch('/api/admin/kelas/create', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ nama: name })
        });
        const result = await res.json();
        if (res.ok && result.success !== false) {
            if (msg) { msg.textContent = 'Kelas berhasil dibuat.'; msg.className = 'text-green-600'; }
            document.getElementById('newKelasName').value = '';
            // refresh kelas dropdowns
            loadKelasDropdown('Kelas');
            loadKelasDropdown('kelasAdminDropdown');
            loadKelasDropdown('kategoriForSubkategori');
        } else {
            throw new Error(result.message || 'Gagal membuat kelas');
        }
    } catch (err) {
        console.error('createKelas error:', err);
        if (msg) { msg.textContent = 'Gagal membuat kelas: ' + err.message; msg.className = 'text-red-600'; }
    }
}

async function deleteKelas() {
    const sel = document.getElementById('kelasAdminDropdown');
    const msg = document.getElementById('kelasMessage');
    if (!sel) return;
    const name = (sel.value || '').trim();
    if (!name) {
        if (msg) { msg.textContent = 'Pilih kelas untuk dihapus.'; msg.className = 'text-red-600'; }
        return;
    }
    if (!confirm('Yakin ingin menghapus kelas: ' + name + '?')) return;

    try {
        const res = await fetch('/api/admin/kelas/delete', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ nama: name })
        });
        const result = await res.json();
        if (res.ok && result.success !== false) {
            if (msg) { msg.textContent = 'Kelas berhasil dihapus.'; msg.className = 'text-green-600'; }
            // refresh kelas dropdowns
            loadKelasDropdown('Kelas');
            loadKelasDropdown('kelasAdminDropdown');
            loadKelasDropdown('kategoriForSubkategori');
        } else {
            throw new Error(result.message || 'Gagal menghapus kelas');
        }
    } catch (err) {
        console.error('deleteKelas error:', err);
        if (msg) { msg.textContent = 'Gagal menghapus kelas: ' + err.message; msg.className = 'text-red-600'; }
    }
}

function showTab(tabId) {
    // Hide all tabs
    document.querySelectorAll('.spa-tab').forEach(tab => {
        tab.classList.add('hidden');
    });
    
    // Show selected tab
    document.getElementById(tabId).classList.remove('hidden');
    
    // Update active nav button
    document.querySelectorAll('.nav-btn').forEach(btn => {
        btn.classList.remove('bg-white/20');
    });
    if (event && event.target) {
        event.target.closest('.nav-btn')?.classList.add('bg-white/20');
    }
    
    // Close sidebar on mobile
    if (window.innerWidth < 1024) {
        const sidebar = document.getElementById('sidebar');
        if (!sidebar.classList.contains('-translate-x-full')) {
            toggleSidebar();
        }
    }

    // Load data when switching tabs
    if (tabId === 'spa-users') {
        loadUserList();
    } else if (tabId === 'spa-bk') {
        // load BK management data
        loadBKList();
        if (document.getElementById('newBkKelas')) loadKelasDropdown('newBkKelas', true);
    } else if (tabId === 'spa-kategori') {
        loadKategoriDropdown('kategoriDropdown');
        loadKategoriDropdown('kategoriForSubkategori');
    }
}

function toggleSection(sectionId) {
    const section = document.getElementById(sectionId);
    section.classList.toggle('hidden');
}

// ==========================================
// DASHBOARD STATISTICS
// ==========================================

function loadDashboardStats() {
    fetch("/api/ext/stats")
        .then(response => {
            if (!response.ok) throw new Error("Network response was not ok");
            return response.json();
        })
        .then(data => {
            const match = data.most_selected_category?.match(/^\w+/);
            const code = match ? match[0] : "N/A";
            document.getElementById("masalah-terberat").textContent = code;
            document.getElementById("total-siswa").textContent = data.total_accounts;
            document.getElementById("total-mengisi").textContent = data.total_submissions;
            document.getElementById("total-belum-mengisi").textContent = data.total_accounts - data.total_submissions;
            document.getElementById("jumlah-kategori").textContent = data.total_category;
            document.getElementById("jumlah-sub-kategori").textContent = data.total_sub_category;
        })
        .catch(error => {
            console.error("Fetch error:", error);
        });
}

// ==========================================
// USER MANAGEMENT FUNCTIONS
// ==========================================

// Client-side user list and pagination state
let adminUsersAll = []; // full list fetched from server
let adminFilteredUsers = []; // after applying search & kelas filter
let adminUserPage = 1;
let adminUserPageSize = 10;

function clearUserInputs() {
    document.getElementById('nama').value = '';
    document.getElementById('password').value = '';
    document.getElementById('Kelas').value = '';
    const roleEl = document.getElementById('roleSelect');
    if (roleEl) roleEl.value = '';
}

async function saveUser() {
    const nama = (document.getElementById('nama')?.value || '').trim();
    const password = (document.getElementById('password')?.value || '').trim();
    const role = (document.getElementById('roleSelect')?.value || '').trim();

    // collect kelas: support multiple selection for BK
    const kelasEl = document.getElementById('Kelas');
    let kelasVal = '';
    if (kelasEl) {
        if (kelasEl.multiple) {
            // array of selected values
            kelasVal = Array.from(kelasEl.selectedOptions).map(o => o.value).filter(Boolean);
        } else {
            kelasVal = (kelasEl.value || '').trim();
        }
    }

    if (!nama || !password || !role) {
        alert("Mohon isi semua field (termasuk role)!");
        return;
    }

    const payload = {
        nama: nama,
        password: password,
        role: role,
        kelas: kelasVal
    };

    try {
        const response = await fetch('/api/admin/user/create', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(payload)
        });

        if (response.ok) {
            alert("User berhasil didaftarkan!");
            clearUserInputs();
            loadUserList();
        } else {
            const err = await response.json().catch(() => ({}));
            alert("Gagal mendaftarkan user: " + (err.message || response.statusText));
        }
    } catch (error) {
        console.error('Error:', error);
        alert("Terjadi kesalahan saat mendaftarkan user.");
    }
}

async function loadUserList() {
    const container = document.getElementById("userListContainer");
    try {
        const response = await fetch("/api/admin/user/read");
        adminUsersAll = await response.json();

        // initialize filters and render
        adminUserPage = 1;
        applyUserFilters();
    } catch (error) {
        console.error('Error:', error);
        if (container) container.innerHTML = "<p class='text-red-600'>Terjadi kesalahan saat mengambil data.</p>";
    }
}
function renderUserPage() {
    const tbody = document.getElementById('userTableBody');
    const pageInfo = document.getElementById('userPageInfo');
    if (!tbody) return;

    const start = (adminUserPage - 1) * adminUserPageSize;
    const pageItems = adminFilteredUsers.slice(start, start + adminUserPageSize);

    let html = '';
    pageItems.forEach((user, idx) => {
        const escapedNama = escapeHtml((user.nama||'').trim());
        const kelasStr = Array.isArray(user.kelas) ? (user.kelas.join(', ')) : (user.kelas || '');
        const escapedKelas = escapeHtml(kelasStr);
        const escapedPassword = escapeHtml(user.password || '-');
        
        html += `
            <tr class="hover:bg-slate-50">
                <td class="px-4 py-3 text-sm text-slate-600">${start + idx + 1}</td>
                <td class="px-4 py-3 text-sm text-slate-800 font-medium">${escapedNama}</td>
                <td class="px-4 py-3 text-sm text-slate-600">${escapedKelas}</td>
                <td class="px-4 py-3 text-sm text-slate-600">${escapedPassword}</td>
                <td class="px-4 py-3 text-sm text-right">
                    <button onclick='deleteUserRow(${JSON.stringify(user.nama)}, ${JSON.stringify(user.kelas)}, ${JSON.stringify(user.password)})' class="px-3 py-1 text-white bg-red-500 hover:bg-red-600 rounded">Hapus</button>
                </td>
            </tr>
        `;
    });

    tbody.innerHTML = html || '<tr><td colspan="5" class="p-4 text-slate-600">Tidak ada data</td></tr>';

    const totalPages = Math.max(1, Math.ceil(adminFilteredUsers.length / adminUserPageSize));
    if (pageInfo) pageInfo.textContent = `Page ${adminUserPage} / ${totalPages}`;
}
function changeUserPageSize() {
    const el = document.getElementById('userPageSize');
    if (!el) return;
    adminUserPageSize = parseInt(el.value, 10) || 10;
    adminUserPage = 1;
    renderUserPage();
}

function prevUserPage() {
    if (adminUserPage > 1) {
        adminUserPage--;
        renderUserPage();
    }
}

function nextUserPage() {
    const totalPages = Math.max(1, Math.ceil(adminFilteredUsers.length / adminUserPageSize));
    if (adminUserPage < totalPages) {
        adminUserPage++;
        renderUserPage();
    }
}

// Delete a user by name/kelas/password (called from per-row button)
async function deleteUserRow(nama, kelas, password) {
    if (!confirm(`Yakin ingin menghapus akun "${nama}" dari kelas "${kelas}"?`)) return;

    try {
        const res = await fetch('/api/admin/user/delete', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ nama: nama, kelas: kelas, password: password })
        });

        if (res.ok) {
            // refresh list
            await loadUserList();
            alert(`Akun "${nama}" berhasil dihapus.`);
        } else {
            const err = await res.json().catch(() => ({}));
            throw new Error(err.message || 'Gagal menghapus akun');
        }
    } catch (err) {
        console.error('deleteUserRow error:', err);
        alert('Gagal menghapus akun: ' + err.message);
    }
}

async function deleteUser() {
    const nameToDelete = document.getElementById('searchName').value.trim();
    const messageElement = document.getElementById('deleteMessage');

    if (!nameToDelete) {
        messageElement.textContent = "Nama harus diisi.";
        messageElement.className = "text-red-600 font-semibold";
        return;
    }

    messageElement.textContent = "Mencari akun...";
    messageElement.className = "text-blue-600 font-semibold";

    try {
        const response = await fetch('/api/admin/user/read');
        const users = await response.json();
        const user = users.find(u => u.nama.trim().toLowerCase() === nameToDelete.toLowerCase());

        if (!user) {
            messageElement.textContent = "User tidak ditemukan.";
            messageElement.className = "text-red-600 font-semibold";
            return;
        }

        messageElement.textContent = `Akun "${user.nama}" ditemukan. Menghapus...`;
        messageElement.className = "text-blue-600 font-semibold";

        const deleteResponse = await fetch('/api/delete/user', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                nama: user.nama,
                kelas: user.kelas,
                password: user.password
            })
        });

        if (deleteResponse.ok) {
            messageElement.textContent = `Akun "${user.nama}" berhasil dihapus.`;
            messageElement.className = "text-green-600 font-semibold";
            document.getElementById('searchName').value = '';
            loadUserList();
        } else {
            messageElement.textContent = `Gagal menghapus akun "${user.nama}".`;
            messageElement.className = "text-red-600 font-semibold";
        }
    } catch (err) {
        console.error(err);
        messageElement.textContent = "Terjadi kesalahan saat menghapus user.";
        messageElement.className = "text-red-600 font-semibold";
    }
}

// ==========================================
// KATEGORI MANAGEMENT FUNCTIONS
// ==========================================

function clearKategoriInputs() {
    document.getElementById("inputKategoriCode").value = "";
    document.getElementById("inputKategori").value = "";
    document.getElementById("kategoriForSubkategori").value = "";
    document.getElementById("inputSubkategori").value = "";
}

async function loadKategoriDropdown(dropdownId) {
    const dropdown = document.getElementById(dropdownId);
    if (!dropdown) return;

    dropdown.innerHTML = '<option value="">-- Pilih Kategori --</option>';

    try {
        const response = await fetch("/api/read/total_kategori");
        if (!response.ok) throw new Error("Failed to fetch kategori");

        const kategoris = await response.json();

        kategoris.forEach(k => {
            const opt = document.createElement("option");
            opt.value = k.code || k.nama_bidang_masalah.substring(0, 3);
            opt.textContent = k.nama_bidang_masalah || "Kategori";
            dropdown.appendChild(opt);
        });
    } catch (err) {
        console.error(err);
        const errorOpt = document.createElement("option");
        errorOpt.textContent = "Gagal memuat kategori";
        errorOpt.disabled = true;
        dropdown.appendChild(errorOpt);
    }
}

async function loadSubkategoriDropdown(kategoriCode, dropdownId) {
    const dropdown = document.getElementById(dropdownId);
    if (!dropdown) return;

    dropdown.innerHTML = '<option value="">-- Pilih Subkategori --</option>';
    dropdown.disabled = true;

    if (!kategoriCode) {
        dropdown.disabled = true;
        return;
    }

    try {
        const response = await fetch(`/api/read-subkategori?kategoriCode=${encodeURIComponent(kategoriCode)}`);
        if (!response.ok) throw new Error("Failed to fetch subkategori");

        const subkategories = await response.json();

        if (subkategories.length === 0) {
            const opt = document.createElement("option");
            opt.value = "";
            opt.textContent = "Tidak ada subkategori";
            opt.disabled = true;
            dropdown.appendChild(opt);
        } else {
            subkategories.forEach(sub => {
                const option = document.createElement("option");
                option.value = sub.nama_soal_masalah;
                option.textContent = sub.nama_soal_masalah || "Subkategori";
                dropdown.appendChild(option);
            });
        }
        dropdown.disabled = false;
    } catch (err) {
        console.error(err);
        const errorOpt = document.createElement("option");
        errorOpt.textContent = "Gagal memuat subkategori";
        errorOpt.disabled = true;
        dropdown.appendChild(errorOpt);
        dropdown.disabled = true;
    }
}

async function saveKategoriAtauSub() {
    const kategoriCodeVal = document.getElementById("inputKategoriCode").value.trim().toUpperCase();
    const kategoriVal = document.getElementById("inputKategori").value.trim();
    const selectedKategoriForSub = document.getElementById("kategoriForSubkategori").value;
    const subkategoriVal = document.getElementById("inputSubkategori").value.trim();

    if (!kategoriCodeVal && !kategoriVal && !subkategoriVal) {
        alert("Mohon isi Kode & Nama Kategori atau Subkategori");
        return;
    }

    if (kategoriCodeVal && kategoriVal && !selectedKategoriForSub) {
        // Validate code is exactly 3 characters
        if (kategoriCodeVal.length !== 3) {
            alert("Kode kategori harus tepat 3 karakter!");
            return;
        }

        // Save new kategori
        try {
            const response = await fetch("/api/save", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({ 
                    code: kategoriCodeVal,
                    kategori: kategoriVal 
                })
            });
            
            const result = await response.json();
            
            if (response.ok && result.success) {
                alert("Kategori berhasil disimpan dengan kode: " + kategoriCodeVal);
                clearKategoriInputs();
                loadKategoriDropdown("kategoriDropdown");
                loadKategoriDropdown("kategoriDeleteDropdown");
                loadKategoriDropdown("kategoriForSubkategori");
            } else {
                alert("Gagal menyimpan kategori: " + (result.message || "Unknown error"));
            }
        } catch (err) {
            console.error(err);
            alert("Gagal menyimpan kategori: " + err.message);
        }
    } else if (selectedKategoriForSub && subkategoriVal) {
        // Save subkategori ke kategori yang dipilih
        try {
            const response = await fetch("/api/save-subkategori", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({ 
                    kategori_code: selectedKategoriForSub, 
                    subkategori: subkategoriVal 
                })
            });
            
            const result = await response.json();
            
            if (response.ok && result.success) {
                alert("Subkategori berhasil disimpan!");
                clearKategoriInputs();
                loadSubkategoriDropdown(selectedKategoriForSub, "subkategoriDropdown");
                loadSubkategoriDropdown(selectedKategoriForSub, "subkategoriDeleteDropdown");
            } else {
                alert("Gagal menyimpan subkategori: " + (result.message || "Unknown error"));
            }
        } catch (err) {
            console.error(err);
            alert("Gagal menyimpan subkategori: " + err.message);
        }
    } else {
        alert("Untuk menambah kategori: isi kode (3 digit) dan nama kategori.\nUntuk menambah subkategori: pilih kategori dan isi nama subkategori.");
    }
}

async function deleteKategori() {
    const selectedCode = document.getElementById("kategoriDeleteDropdown").value;
    const messageElement = document.getElementById("deleteKategoriMessage");

    if (!selectedCode) {
        messageElement.textContent = "Pilih kategori yang akan dihapus.";
        messageElement.className = "text-red-600 font-semibold";
        return;
    }

    if (!confirm("Yakin ingin menghapus kategori ini? Semua subkategori terkait juga akan dihapus.")) {
        return;
    }

    messageElement.textContent = "Menghapus kategori...";
    messageElement.className = "text-blue-600 font-semibold";

    try {
        const response = await fetch("/admin/kategori/delete", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ kategori_code: selectedCode })
        });

        const result = await response.json();

        if (response.ok && result.success) {
            messageElement.textContent = "Kategori berhasil dihapus.";
            messageElement.className = "text-green-600 font-semibold";

            loadKategoriDropdown("kategoriDropdown");
            loadKategoriDropdown("kategoriDeleteDropdown");
            loadKategoriDropdown("kategoriForSubkategori");
            document.getElementById('subkategoriDeleteDropdown').innerHTML = '<option value="">-- Pilih Subkategori untuk Hapus --</option>';
            document.getElementById('subkategoriDeleteDropdown').disabled = true;
        } else {
            messageElement.textContent = "Gagal menghapus kategori: " + (result.message || "Unknown error");
            messageElement.className = "text-red-600 font-semibold";
        }
    } catch (err) {
        console.error(err);
        messageElement.textContent = "Terjadi kesalahan saat menghapus kategori: " + err.message;
        messageElement.className = "text-red-600 font-semibold";
    }
}

async function deleteSubkategori() {
    const kategoriCode = document.getElementById("kategoriDeleteDropdown").value;
    const subkategoriName = document.getElementById("subkategoriDeleteDropdown").value;
    const messageElement = document.getElementById("deleteKategoriMessage");

    if (!kategoriCode || !subkategoriName) {
        messageElement.textContent = "Pilih kategori dan subkategori yang akan dihapus.";
        messageElement.className = "text-red-600 font-semibold";
        return;
    }

    if (!confirm("Yakin ingin menghapus subkategori ini?")) {
        return;
    }

    messageElement.textContent = "Menghapus subkategori...";
    messageElement.className = "text-blue-600 font-semibold";

    try {
        const response = await fetch("/admin/subkategori/delete", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ subkategori_name: subkategoriName })
        });

        const result = await response.json();

        if (response.ok && result.success) {
            messageElement.textContent = "Subkategori berhasil dihapus.";
            messageElement.className = "text-green-600 font-semibold";

            loadSubkategoriDropdown(kategoriCode, "subkategoriDropdown");
            loadSubkategoriDropdown(kategoriCode, "subkategoriDeleteDropdown");
        } else {
            messageElement.textContent = "Gagal menghapus subkategori: " + (result.message || "Unknown error");
            messageElement.className = "text-red-600 font-semibold";
        }
    } catch (err) {
        console.error(err);
        messageElement.textContent = "Terjadi kesalahan saat menghapus subkategori: " + err.message;
        messageElement.className = "text-red-600 font-semibold";
    }
}

// ==========================================
// EVENT LISTENERS & INITIALIZATION
// ==========================================

document.addEventListener('DOMContentLoaded', () => {
    // Load dashboard stats on page load
    loadDashboardStats();

    // Auto uppercase for code input
    const codeInput = document.getElementById("inputKategoriCode");
    if (codeInput) {
        codeInput.addEventListener("input", function() {
            this.value = this.value.toUpperCase();
        });
    }

    // Setup kategori dropdown change listeners
    const kategoriDropdown = document.getElementById('kategoriDropdown');
    if (kategoriDropdown) {
        kategoriDropdown.addEventListener('change', () => {
            const kategoriCode = kategoriDropdown.value;
            loadSubkategoriDropdown(kategoriCode, 'subkategoriDropdown');
        });
    }

    const kategoriDeleteDropdown = document.getElementById('kategoriDeleteDropdown');
    if (kategoriDeleteDropdown) {
        kategoriDeleteDropdown.addEventListener('change', () => {
            const kategoriCode = kategoriDeleteDropdown.value;
            loadSubkategoriDropdown(kategoriCode, 'subkategoriDeleteDropdown');
            // Clear message when changing selection
            const msgEl = document.getElementById('deleteKategoriMessage');
            if (msgEl) msgEl.textContent = '';
        });
    }

    // Load kelas dropdown for pages that have a Kelas select
    if (document.getElementById('Kelas')) {
        loadKelasDropdown('Kelas');
    }
    // Also populate any generic kelasSelect used on other pages
    if (document.getElementById('kelasSelect')) {
        loadKelasDropdown('kelasSelect', true);
    }
    // populate admin kelas dropdown for management
    if (document.getElementById('kelasAdminDropdown')) {
        loadKelasDropdown('kelasAdminDropdown', true);
    }
    // populate kelas filter for user list and adjust its placeholder to allow 'all'
    if (document.getElementById('userFilterKelas')) {
        loadKelasDropdown('userFilterKelas', true).then(() => {
            const uf = document.getElementById('userFilterKelas');
            if (uf && uf.options && uf.options.length > 0) {
                // make first option a usable 'all' option
                uf.options[0].disabled = false;
                uf.options[0].selected = true;
                uf.options[0].value = '';
                uf.options[0].text = '-- Semua Kelas --';
            }
        }).catch(err => { console.error(err); });
    }

    // populate role dropdown for register form
    if (document.getElementById('roleSelect')) {
        loadRoleDropdown('roleSelect', true).then(() => {
            const roleEl = document.getElementById('roleSelect');
            const kelasEl = document.getElementById('Kelas');
            // toggle Kelas select to allow multiple selection for BK
            roleEl.addEventListener('change', () => {
                if (!kelasEl) return;
                const roleVal = (roleEl.value || '').toLowerCase();
                if (roleVal === 'bk') {
                    kelasEl.multiple = true;
                    // make it easier to select multiple
                    kelasEl.size = Math.min(8, Math.max(4, kelasEl.options.length));
                    if (kelasEl.options && kelasEl.options.length > 0) {
                        kelasEl.options[0].disabled = false;
                        kelasEl.options[0].text = '-- Pilih Kelas (Ctrl/Cmd+click untuk multi) --';
                    }
                } else {
                    // switch back to single-select
                    kelasEl.multiple = false;
                    kelasEl.size = 1;
                    if (kelasEl.options && kelasEl.options.length > 0) {
                        kelasEl.options[0].text = '-- Pilih Kelas --';
                    }
                }
            });
        }).catch(err => { console.error(err); });
    }
    // populate newBkKelas (used in BK create form)
    if (document.getElementById('newBkKelas')) {
        loadKelasDropdown('newBkKelas', true).then(() => {
            const nb = document.getElementById('newBkKelas');
            if (nb) {
                // allow multiple assignment at creation time
                nb.multiple = true;
                nb.size = Math.min(8, Math.max(4, nb.options.length));
            }
        }).catch(err => { console.error(err); });
    }
});

// Apply filters (search + kelas) to the full user list and render pagination
function applyUserFilters() {
    const q = (document.getElementById('userSearchInput')?.value || '').trim().toLowerCase();
    const kelas = (document.getElementById('userFilterKelas')?.value || '').trim();

    adminFilteredUsers = adminUsersAll.filter(u => {
        // filter by kelas if selected
        if (kelas && (u.kelas || '').trim() !== kelas) return false;

        if (!q) return true;
        // search in nama, kelas, password
        const name = (u.nama || '').toLowerCase();
        const k = (u.kelas || '').toLowerCase();
        const p = (u.password || '').toLowerCase();
        return name.includes(q) || k.includes(q) || p.includes(q);
    });

    adminUserPage = 1;
    renderUserPage();
}

// Fetch kelas list from the API and populate a select element
async function loadKelasDropdown(dropdownId, includePlaceholder = true) {
    const dropdown = document.getElementById(dropdownId);
    if (!dropdown) return;

    // Preserve a placeholder option
    dropdown.innerHTML = '';
    if (includePlaceholder) {
        const ph = document.createElement('option');
        ph.value = '';
        ph.textContent = dropdownId === 'Kelas' ? '-- Pilih Kelas --' : '-- Pilih Kelas --';
        ph.disabled = true;
        ph.selected = true;
        dropdown.appendChild(ph);
    }

    try {
        const res = await fetch('/api/admin/kelas/read');
        if (!res.ok) throw new Error('Failed to fetch kelas');
        const data = await res.json();

        // Sort by nama for nicer UX
        data.sort((a, b) => (a.nama || '').localeCompare(b.nama || ''));

        data.forEach(item => {
            const opt = document.createElement('option');
            opt.value = item.nama;
            opt.textContent = item.nama;
            dropdown.appendChild(opt);
        });
    } catch (err) {
        console.error('loadKelasDropdown error:', err);
        const errOpt = document.createElement('option');
        errOpt.textContent = 'Gagal memuat kelas';
        errOpt.disabled = true;
        dropdown.appendChild(errOpt);
    }
}

// Fetch roles list from the API and populate a select element
async function loadRoleDropdown(dropdownId, includePlaceholder = true) {
    const dropdown = document.getElementById(dropdownId);
    if (!dropdown) return;

    dropdown.innerHTML = '';
    if (includePlaceholder) {
        const ph = document.createElement('option');
        ph.value = '';
        ph.textContent = '-- Pilih Role --';
        ph.disabled = true;
        ph.selected = true;
        dropdown.appendChild(ph);
    }

    try {
        const res = await fetch('/api/ext/roles');
        if (!res.ok) throw new Error('Failed to fetch roles');
        const data = await res.json();

        // Sort by nama
        data.sort((a, b) => (a.nama || '').localeCompare(b.nama || ''));

        data.forEach(item => {
            const opt = document.createElement('option');
            opt.value = item.nama;
            opt.textContent = item.nama;
            dropdown.appendChild(opt);
        });
    } catch (err) {
        console.error('loadRoleDropdown error:', err);
        const errOpt = document.createElement('option');
        errOpt.textContent = 'Gagal memuat roles';
        errOpt.disabled = true;
        dropdown.appendChild(errOpt);
    }
}
// ==========================
// BK MANAGEMENT (Admin UI) - UPDATED TO MATCH BACKEND
// ==========================

// Load BK counselors with their assigned classes
async function loadBKList() {
    try {
        const res = await fetch('/api/admin/bk-duties/list');
        if (!res.ok) throw new Error('Failed to fetch BK duties');
        const counselors = await res.json();
        renderBKList(counselors);
    } catch (err) {
        console.error('loadBKList error:', err);
        const tbody = document.getElementById('bkTableBody');
        if (tbody) tbody.innerHTML = '<tr><td colspan="4" class="p-4 text-red-600">Gagal memuat daftar BK.</td></tr>';
    }
}

// Render BK counselors list
async function renderBKList(counselors) {
    const tbody = document.getElementById('bkTableBody');
    if (!tbody) return;
    
    let html = '';
    counselors.forEach((counselor, idx) => {
        const nama = escapeHtml(counselor.counselor_name || '');
        const assignedClasses = counselor.assigned_classes || [];
        const kelasList = assignedClasses.map(c => c.nama).join(', ');
        
        html += `
            <tr class="hover:bg-slate-50">
                <td class="px-4 py-3 text-sm text-slate-600">${idx + 1}</td>
                <td class="px-4 py-3 text-sm text-slate-800 font-medium">${nama}</td>
                <td class="px-4 py-3 text-sm text-slate-600">${escapeHtml(kelasList) || '-'}</td>
                <td class="px-4 py-3 text-sm text-right space-x-2">
                    <select id="bk-kelas-${counselor.user_id}" multiple size="4" class="px-2 py-1 border rounded text-sm">
                        <option value="">-- Pilih Kelas --</option>
                    </select>
                    <button onclick="assignBKClass(${counselor.user_id})" class="px-3 py-1 text-white bg-green-500 hover:bg-green-600 rounded text-sm">Tambah</button>
                    <button onclick="removeBKClass(${counselor.user_id})" class="px-3 py-1 text-white bg-red-500 hover:bg-red-600 rounded text-sm">Hapus</button>
                </td>
            </tr>
        `;
    });

    tbody.innerHTML = html || '<tr><td colspan="4" class="p-4 text-slate-600">Tidak ada BK terdaftar</td></tr>';

    // Populate kelas dropdowns
    try {
        const res = await fetch('/api/admin/kelas/read');
        if (res.ok) {
            const kelasList = await res.json();
            kelasList.sort((a,b) => (a.nama||'').localeCompare(b.nama||''));
            
            counselors.forEach(counselor => {
                const sel = document.getElementById(`bk-kelas-${counselor.user_id}`);
                if (!sel) return;
                
                // Clear and add options
                sel.innerHTML = '<option value="">-- Pilih Kelas --</option>';
                const assignedClassIds = (counselor.assigned_classes || []).map(c => c.id);
                
                kelasList.forEach(kelas => {
                    const opt = document.createElement('option');
                    opt.value = kelas.id;
                    opt.textContent = kelas.nama;
                    // Don't pre-select assigned classes for addition
                    sel.appendChild(opt);
                });
            });
        }
    } catch (err) {
        console.error('populate BK kelas error:', err);
    }
}

// Assign classes to BK counselor
async function assignBKClass(userId) {
    const sel = document.getElementById(`bk-kelas-${userId}`);
    if (!sel) return;
    
    const kelasIds = Array.from(sel.selectedOptions)
        .map(o => o.value)
        .filter(Boolean)
        .map(id => parseInt(id));
    
    if (kelasIds.length === 0) {
        alert('Pilih minimal satu kelas untuk ditambahkan');
        return;
    }

    if (!confirm(`Tambahkan ${kelasIds.length} kelas untuk BK ini?`)) return;

    try {
        const res = await fetch('/api/admin/bk-duties/assign', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ 
                user_id: userId,
                kelas_ids: kelasIds
            })
        });
        
        const result = await res.json();
        
        if (res.ok && result.success) {
            alert('Kelas berhasil ditambahkan untuk BK');
            
            // Show assignment results
            if (result.failed_assignments.length > 0) {
                alert(`Berhasil: ${result.successful_assignments.join(', ')}\nGagal: ${result.failed_assignments.join(', ')}`);
            }
            
            loadBKList();
        } else {
            throw new Error(result.error || 'Gagal menambahkan kelas');
        }
    } catch (err) {
        console.error('assignBKClass error:', err);
        alert('Gagal menambahkan kelas: ' + err.message);
    }
}

// Remove classes from BK counselor
async function removeBKClass(userId) {
    const sel = document.getElementById(`bk-kelas-${userId}`);
    if (!sel) return;
    
    const kelasIds = Array.from(sel.selectedOptions)
        .map(o => o.value)
        .filter(Boolean)
        .map(id => parseInt(id));
    
    if (kelasIds.length === 0) {
        alert('Pilih minimal satu kelas untuk dihapus');
        return;
    }

    if (!confirm(`Hapus ${kelasIds.length} kelas dari BK ini?`)) return;

    try {
        const res = await fetch('/api/admin/bk-duties/remove', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ 
                user_id: userId,
                kelas_ids: kelasIds
            })
        });
        
        const result = await res.json();
        
        if (res.ok && result.success) {
            alert('Kelas berhasil dihapus dari BK');
            
            // Show removal results
            if (result.failed_removals.length > 0) {
                alert(`Berhasil: ${result.successful_removals.join(', ')}\nGagal: ${result.failed_removals.join(', ')}`);
            }
            
            loadBKList();
        } else {
            throw new Error(result.error || 'Gagal menghapus kelas');
        }
    } catch (err) {
        console.error('removeBKClass error:', err);
        alert('Gagal menghapus kelas: ' + err.message);
    }
}

// Create new BK counselor account
async function createBK() {
    const nama = (document.getElementById('newBkName')?.value || '').trim();
    const password = (document.getElementById('newBkPassword')?.value || '').trim();
    const newKEl = document.getElementById('newBkKelas');
    
    let kelasIds = [];
    if (newKEl && newKEl.multiple) {
        kelasIds = Array.from(newKEl.selectedOptions)
            .map(o => o.value)
            .filter(Boolean)
            .map(id => parseInt(id));
    }

    if (!nama || !password) {
        alert('Isi nama dan password untuk membuat BK.');
        return;
    }

    try {
        // First create the BK user account
        const createRes = await fetch('/api/admin/user/create', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ 
                nama: nama, 
                password: password, 
                role: 'BK'
            })
        });

        if (!createRes.ok) {
            const err = await createRes.json().catch(() => ({}));
            throw new Error(err.message || 'Gagal membuat akun BK');
        }

        const userResult = await createRes.json();
        const userId = userResult.user_id;

        // If kelas are selected, assign them
        if (kelasIds.length > 0 && userId) {
            const assignRes = await fetch('/api/admin/bk-duties/assign', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ 
                    user_id: userId,
                    kelas_ids: kelasIds
                })
            });

            const assignResult = await assignRes.json();
            
            if (!assignRes.ok || !assignResult.success) {
                console.warn('BK created but class assignment failed:', assignResult);
            }
        }

        alert('BK berhasil dibuat' + (kelasIds.length > 0 ? ' dengan kelas terpasang' : ''));
        
        // Clear form
        document.getElementById('newBkName').value = '';
        document.getElementById('newBkPassword').value = '';
        if (newKEl) newKEl.value = '';
        
        // Refresh lists
        loadBKList();
        loadUserList();
        
    } catch (err) {
        console.error('createBK error:', err);
        alert('Gagal membuat BK: ' + err.message);
    }
}

// Mark user as BK (alternative single assignment)
async function markUserAsBK(userId, kelasId, isActive = true) {
    try {
        const res = await fetch('/api/admin/users/mark-bk', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ 
                user_id: userId,
                kelas_id: kelasId,
                is_active: isActive
            })
        });
        
        const result = await res.json();
        
        if (res.ok) {
            return result;
        } else {
            throw new Error(result.error || 'Gagal menandai user sebagai BK');
        }
    } catch (err) {
        console.error('markUserAsBK error:', err);
        throw err;
    }
}

// Update BK management tab initialization
function showTab(tabId) {
    // Hide all tabs
    document.querySelectorAll('.spa-tab').forEach(tab => {
        tab.classList.add('hidden');
    });
    
    // Show selected tab
    document.getElementById(tabId).classList.remove('hidden');
    
    // Update active nav button
    document.querySelectorAll('.nav-btn').forEach(btn => {
        btn.classList.remove('bg-white/20');
    });
    if (event && event.target) {
        event.target.closest('.nav-btn')?.classList.add('bg-white/20');
    }
    
    // Close sidebar on mobile
    if (window.innerWidth < 1024) {
        const sidebar = document.getElementById('sidebar');
        if (!sidebar.classList.contains('-translate-x-full')) {
            toggleSidebar();
        }
    }

    // Load data when switching tabs
    if (tabId === 'spa-users') {
        loadUserList();
    } else if (tabId === 'spa-bk') {
        loadBKList();
        if (document.getElementById('newBkKelas')) loadKelasDropdown('newBkKelas', true);
    } else if (tabId === 'spa-kategori') {
        loadKategoriDropdown('kategoriDropdown');
        loadKategoriDropdown('kategoriForSubkategori');
    }
}
