/*M!999999\- enable the sandbox mode */ 
-- MariaDB dump 10.19-12.0.2-MariaDB, for Linux (x86_64)
--
-- Host: localhost    Database: aum_db
-- ------------------------------------------------------
-- Server version	12.0.2-MariaDB

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*M!100616 SET @OLD_NOTE_VERBOSITY=@@NOTE_VERBOSITY, NOTE_VERBOSITY=0 */;

--
-- Table structure for table `bidang_masalah`
--

DROP TABLE IF EXISTS `bidang_masalah`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `bidang_masalah` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `nama_bidang_masalah` varchar(255) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `nama_bidang_masalah` (`nama_bidang_masalah`)
) ENGINE=InnoDB AUTO_INCREMENT=16 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `bidang_masalah`
--

LOCK TABLES `bidang_masalah` WRITE;
/*!40000 ALTER TABLE `bidang_masalah` DISABLE KEYS */;
set autocommit=0;
INSERT INTO `bidang_masalah` VALUES
(13,'ANM(Agama dan Nilai Moral)'),
(7,'DPI(Diri Pribadi)'),
(11,'EDK(Ekonomi & Keuangan)'),
(14,'HMP(Hubungan Muda-mudi dan Perkawinan)'),
(9,'HSO(Hubungan Sosial)'),
(6,'JDK(Jasmani dan kesehatan)'),
(10,'KDP(Karir dan Pekerjaan)'),
(8,'KHK(Keadaan dan Hubungan dalam Keluarga)'),
(15,'PDP(Pendidikan dan Pembelajaran)'),
(12,'WSG(Waktu Senggang)');
/*!40000 ALTER TABLE `bidang_masalah` ENABLE KEYS */;
UNLOCK TABLES;
commit;

--
-- Table structure for table `bk_class_duties`
--

DROP TABLE IF EXISTS `bk_class_duties`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `bk_class_duties` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL,
  `kelas_id` int(11) NOT NULL,
  `is_active` tinyint(1) DEFAULT 1,
  `assigned_at` timestamp NULL DEFAULT current_timestamp(),
  PRIMARY KEY (`id`),
  UNIQUE KEY `unique_assignment` (`user_id`,`kelas_id`),
  KEY `kelas_id` (`kelas_id`),
  KEY `idx_bk_duties_user` (`user_id`,`is_active`),
  CONSTRAINT `bk_class_duties_ibfk_1` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`) ON DELETE CASCADE,
  CONSTRAINT `bk_class_duties_ibfk_2` FOREIGN KEY (`kelas_id`) REFERENCES `kelas` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `bk_class_duties`
--

LOCK TABLES `bk_class_duties` WRITE;
/*!40000 ALTER TABLE `bk_class_duties` DISABLE KEYS */;
set autocommit=0;
INSERT INTO `bk_class_duties` VALUES
(1,255,14,1,'2025-10-30 04:24:09'),
(2,255,7,1,'2025-10-30 04:24:09'),
(3,255,12,1,'2025-10-30 04:24:09'),
(4,255,9,1,'2025-10-30 04:24:09');
/*!40000 ALTER TABLE `bk_class_duties` ENABLE KEYS */;
UNLOCK TABLES;
commit;

--
-- Table structure for table `hasil`
--

DROP TABLE IF EXISTS `hasil`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `hasil` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL,
  `soal_masalah_id` int(11) NOT NULL,
  `soal_masalah_kategori` varchar(255) DEFAULT NULL,
  `created_at` timestamp NULL DEFAULT current_timestamp(),
  PRIMARY KEY (`id`),
  KEY `soal_masalah_id` (`soal_masalah_id`),
  KEY `soal_masalah_kategori` (`soal_masalah_kategori`),
  KEY `idx_hasil_user_id` (`user_id`),
  CONSTRAINT `hasil_ibfk_1` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`) ON DELETE CASCADE,
  CONSTRAINT `hasil_ibfk_2` FOREIGN KEY (`soal_masalah_id`) REFERENCES `soal_masalah` (`id`),
  CONSTRAINT `hasil_ibfk_3` FOREIGN KEY (`soal_masalah_kategori`) REFERENCES `bidang_masalah` (`nama_bidang_masalah`)
) ENGINE=InnoDB AUTO_INCREMENT=29 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hasil`
--

LOCK TABLES `hasil` WRITE;
/*!40000 ALTER TABLE `hasil` DISABLE KEYS */;
set autocommit=0;
/*!40000 ALTER TABLE `hasil` ENABLE KEYS */;
UNLOCK TABLES;
commit;

--
-- Table structure for table `is_submitted`
--

DROP TABLE IF EXISTS `is_submitted`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `is_submitted` (
  `user_id` int(11) NOT NULL,
  `submitted_at` timestamp NULL DEFAULT current_timestamp(),
  PRIMARY KEY (`user_id`),
  CONSTRAINT `is_submitted_ibfk_1` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `is_submitted`
--

LOCK TABLES `is_submitted` WRITE;
/*!40000 ALTER TABLE `is_submitted` DISABLE KEYS */;
set autocommit=0;
/*!40000 ALTER TABLE `is_submitted` ENABLE KEYS */;
UNLOCK TABLES;
commit;

--
-- Table structure for table `kelas`
--

DROP TABLE IF EXISTS `kelas`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `kelas` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `nama` varchar(100) NOT NULL,
  `jurusan` varchar(100) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `nama` (`nama`)
) ENGINE=InnoDB AUTO_INCREMENT=28 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `kelas`
--

LOCK TABLES `kelas` WRITE;
/*!40000 ALTER TABLE `kelas` DISABLE KEYS */;
set autocommit=0;
INSERT INTO `kelas` VALUES
(5,'X PPLG 1',NULL),
(7,'X DKV 1',NULL),
(8,'X TJKT 1',NULL),
(9,'X TJKT 2',NULL),
(10,'X TJKT 3',NULL),
(11,'X TJKT 4',NULL),
(12,'X DKV 2',NULL),
(13,'X PPLG 2',NULL),
(14,'X BC',NULL),
(16,'X RPL 1',NULL),
(17,'X RPL 2',NULL),
(19,'XI PPLG 2',NULL),
(20,'XI DKV 2',NULL),
(21,'XI DKV 1',NULL),
(22,'XI TJKT 1',NULL),
(23,'XI TJKT 2',NULL),
(24,'XI TJKT 3',NULL),
(25,'XI TJKT 4',NULL),
(26,'XI BC',NULL),
(27,'XI PPLG 1',NULL);
/*!40000 ALTER TABLE `kelas` ENABLE KEYS */;
UNLOCK TABLES;
commit;

--
-- Table structure for table `roles`
--

DROP TABLE IF EXISTS `roles`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `roles` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `nama` varchar(50) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `nama` (`nama`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `roles`
--

LOCK TABLES `roles` WRITE;
/*!40000 ALTER TABLE `roles` DISABLE KEYS */;
set autocommit=0;
INSERT INTO `roles` VALUES
(1,'Admin'),
(2,'BK'),
(3,'Siswa');
/*!40000 ALTER TABLE `roles` ENABLE KEYS */;
UNLOCK TABLES;
commit;

--
-- Table structure for table `soal_masalah`
--

DROP TABLE IF EXISTS `soal_masalah`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `soal_masalah` (
  `id` int(11) NOT NULL,
  `nama_bidang_masalah` varchar(255) NOT NULL,
  `nama_soal_masalah` text NOT NULL,
  PRIMARY KEY (`id`),
  KEY `nama_bidang_masalah` (`nama_bidang_masalah`),
  CONSTRAINT `soal_masalah_ibfk_1` FOREIGN KEY (`nama_bidang_masalah`) REFERENCES `bidang_masalah` (`nama_bidang_masalah`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `soal_masalah`
--

LOCK TABLES `soal_masalah` WRITE;
/*!40000 ALTER TABLE `soal_masalah` DISABLE KEYS */;
set autocommit=0;
INSERT INTO `soal_masalah` VALUES
(1,'JDK(Jasmani dan kesehatan)','Badan terlalu kurus atau gemuk'),
(2,'JDK(Jasmani dan kesehatan)','Warna kulit kurang memuaskan'),
(3,'JDK(Jasmani dan kesehatan)','Berat badan terus berkurang atau bertambah'),
(4,'JDK(Jasmani dan kesehatan)','Badan terlalu pendek'),
(5,'JDK(Jasmani dan kesehatan)','Secara jasmaniah kurang menarik'),
(6,'KDP(Karir dan Pekerjaan)','Belum mampu memikirkan dan memilih pekerjaan yang akan dijabat nantinya'),
(7,'KDP(Karir dan Pekerjaan)','Belum mengetahui bakat diri sendiri untuk jabatan/pekerjaan apa'),
(8,'KDP(Karir dan Pekerjaan)','Kurang memiliki pengetahuan yang luas tentang lapangan pekerjaan dan seluk beluk jenis-jenis pekerjaan'),
(9,'KDP(Karir dan Pekerjaan)','Ingin memperoleh bantuan dalam mendapatkan pekerjaan sambilan untuk melatih diri bekerja sambil sekolah'),
(10,'KDP(Karir dan Pekerjaan)','Khawatir akan pekerjaan yang dijabatnya nanti; jangan-jangan memberikan penghasilan yang tidak mencukupi'),
(11,'PDP(Pendidikan dan Pembelajaran)','Terpaksa atau ragu ragu memasuki sekolah ini'),
(12,'PDP(Pendidikan dan Pembelajaran)','Meragukan kemanfaatan memasuki sekolah ini'),
(13,'PDP(Pendidikan dan Pembelajaran)','Sukar menyesuaikan diri dengan keadaan sekolah'),
(14,'PDP(Pendidikan dan Pembelajaran)','Kurang meminati pelajaran atau jurusan atau program yang diikuti'),
(15,'PDP(Pendidikan dan Pembelajaran)','Khawatir tidak dapat menamatkan sekolah pada waktu yang direncanakan'),
(16,'JDK(Jasmani dan kesehatan)','Fungsi dan/atau kondisi kesehatan mata kurang baik'),
(17,'JDK(Jasmani dan kesehatan)','Mengalami gangguan tertentu karena cacat jasmani'),
(18,'JDK(Jasmani dan kesehatan)','Fungsi dan/atau kondisi kesehatan hidung kurang baik'),
(19,'JDK(Jasmani dan kesehatan)','Kondisi kesehatan kulit sering terganggu'),
(20,'JDK(Jasmani dan kesehatan)','Gangguan pada gigi'),
(21,'KDP(Karir dan Pekerjaan)','Ragu akan kemampuan saya untuk sukses dalam bekerja'),
(22,'KDP(Karir dan Pekerjaan)','Belum mampu merencanakan masa depan'),
(23,'KDP(Karir dan Pekerjaan)','Takut akan bayangan masa depan'),
(24,'KDP(Karir dan Pekerjaan)','Mengalami masalah karena membandingan-bandingan pekerjaan yang layak atau tidak layak untuk menjabat'),
(25,'KDP(Karir dan Pekerjaan)','Khawatir diperlakukan secara tidak wajar atau tidak adil dalam mencari dan/atau melamar pekerjaan'),
(26,'PDP(Pendidikan dan Pembelajaran)','Sering tidak masuk sekolah'),
(27,'PDP(Pendidikan dan Pembelajaran)','Tugas-tugas pelajaran tidak selesai pada waktunya'),
(28,'PDP(Pendidikan dan Pembelajaran)','Sukar memahami penjelasan guru sewaktu pelajaran berlangsung'),
(29,'PDP(Pendidikan dan Pembelajaran)','Mengalami kesulitan dalam membuat catatan pelajaran'),
(30,'PDP(Pendidikan dan Pembelajaran)','Terpaksa mengikuti mata pelajaran yang tidak disukai'),
(31,'JDK(Jasmani dan kesehatan)','Fungsi dan/atau kerongkongan kurang baik atau sering terganggu misalnya serak'),
(32,'JDK(Jasmani dan kesehatan)','Gagap dalam berbicara'),
(33,'JDK(Jasmani dan kesehatan)','Fungsi dan/atau kondisi kesehatan telinga kurang baik'),
(34,'JDK(Jasmani dan kesehatan)','Kurang mampu berolahraga karena kondisi jasmani yang kurang baik'),
(35,'JDK(Jasmani dan kesehatan)','Gangguan pada pencernaan makanan'),
(36,'KDP(Karir dan Pekerjaan)','Kurang yakin terhadap kemampuan pendidikan sekarang ini dalam menyiapkan jabatan tertentu nantinya'),
(37,'KDP(Karir dan Pekerjaan)','Ragu tentang kesempatan memperoleh pekerjaan sesuai dengan pendidikan yang diikuti sekarang ini'),
(38,'KDP(Karir dan Pekerjaan)','Ingin mengikuti kegiatan pelajaran dan/atau latihan khusus tertentu yang benar-benar menunjang proses mencari dan melamar pekerjaan setamat pendidikan ini'),
(39,'KDP(Karir dan Pekerjaan)','Cemas kalau menjadi pengangguran setamat pendidikan ini'),
(40,'KDP(Karir dan Pekerjaan)','Ragu apakah setamat pendidikan ini dapat bekerja secara mandiri'),
(41,'PDP(Pendidikan dan Pembelajaran)','Gelisah dan/atau melakukan kegiatan yang tidak menentu sewaktu pelajaran berlangsung misalnya membuat coret-coretan dalam buku cenderung mengganggu teman'),
(42,'PDP(Pendidikan dan Pembelajaran)','Sering malas belajar'),
(43,'PDP(Pendidikan dan Pembelajaran)','Kurang konsentrasi dalam mengikuti pelajaran'),
(44,'PDP(Pendidikan dan Pembelajaran)','Khawatir tugas-tugas pelajaran hasilnya kurang memuaskan atau rendah'),
(45,'PDP(Pendidikan dan Pembelajaran)','Mengalami masalah karena kemajuan atau hasil belajar hanya diberitahukan pada akhir catur wulan'),
(46,'JDK(Jasmani dan kesehatan)','Sering pusing dan/atau mudah sakit'),
(47,'JDK(Jasmani dan kesehatan)','Mengalami gangguan setiap datang bulan'),
(48,'JDK(Jasmani dan kesehatan)','Secara umum merasa tidak sehat'),
(49,'JDK(Jasmani dan kesehatan)','Khawatir mengidap penyakit turunan'),
(50,'JDK(Jasmani dan kesehatan)','Selera makan sering terganggu'),
(51,'PDP(Pendidikan dan Pembelajaran)','Hasil belajar atau nilai-nilai kurang memuaskan'),
(52,'PDP(Pendidikan dan Pembelajaran)','Mengalami masalah dalam belajar kelompok'),
(53,'PDP(Pendidikan dan Pembelajaran)','Kurang berminat dan/atau kurang mampu mempelajari buku pelajaran'),
(54,'PDP(Pendidikan dan Pembelajaran)','Takut dan atau kurang mampu berbicara di dalam kelas dan/atau di luar kelas'),
(55,'PDP(Pendidikan dan Pembelajaran)','Mengalami kesulitan dalam ejaan tata bahasa dan/atau perbendaharaan kata dalam bahasa indonesia'),
(56,'PDP(Pendidikan dan Pembelajaran)','Mengalami masalah dalam menjawab pertanyaan ujian'),
(57,'PDP(Pendidikan dan Pembelajaran)','Tidak mengetahui dan/atau tidak mampu menerapkan cara-cara belajar yang baik'),
(58,'PDP(Pendidikan dan Pembelajaran)','Kekurangan waktu untuk belajar'),
(59,'PDP(Pendidikan dan Pembelajaran)','Mengalami masalah dalam menyusun makalah laporan atau karya tulis lainnya'),
(60,'PDP(Pendidikan dan Pembelajaran)','Sukar mendapatkan buku pelajaran yang diperlukan'),
(61,'JDK(Jasmani dan kesehatan)','Mengidap penyakit kambuhan'),
(62,'JDK(Jasmani dan kesehatan)','Alergi terhadap makanan atau keadaan tertentu'),
(63,'JDK(Jasmani dan kesehatan)','Kurang atau susah tidur'),
(64,'JDK(Jasmani dan kesehatan)','Mengalami gangguan akibat merokok atau minuman atau obat-obatan'),
(65,'JDK(Jasmani dan kesehatan)','Khawatir tertular penyakit yang diderita orang lain'),
(66,'PDP(Pendidikan dan Pembelajaran)','Mengalami kesulitan dalam pemahaman dan penggunaan istilah dan/atau bahasa inggris dan/atau bahasa asing lainnya'),
(67,'PDP(Pendidikan dan Pembelajaran)','Kesulitan dalam membaca cepat dan/atau memahami isi buku pelajaran'),
(68,'PDP(Pendidikan dan Pembelajaran)','Takut menghadapi ulangan/ujian'),
(69,'PDP(Pendidikan dan Pembelajaran)','Khawatir memperoleh nilai rendah dalam ulangan/ujian ataupun tugas-tugas'),
(70,'PDP(Pendidikan dan Pembelajaran)','Kesulitan dalam mengingat materi pelajaran'),
(71,'PDP(Pendidikan dan Pembelajaran)','Sering kali tidak siap menghadapi ujian'),
(72,'PDP(Pendidikan dan Pembelajaran)','Sarana belajar di sekolah kurang memadai'),
(73,'PDP(Pendidikan dan Pembelajaran)','Orang tua kurang peduli dan/atau kurang membantu kegiatan belajar di sekolah dan/atau di rumah'),
(74,'PDP(Pendidikan dan Pembelajaran)','Anggota keluarga kurang peduli dan/atau kurang membantu kegiatan belajar di sekolah dan/atau di rumah'),
(75,'PDP(Pendidikan dan Pembelajaran)','Sarana belajar di rumah kurang memadai'),
(76,'DPI(Diri Pribadi)','Sering mimpi buruk'),
(77,'DPI(Diri Pribadi)','Cemas atau khawatir tentang sesuatu yang belum pasti'),
(78,'DPI(Diri Pribadi)','Mudah lupa'),
(79,'DPI(Diri Pribadi)','Sering melamun atau berkhayal'),
(80,'DPI(Diri Pribadi)','Ceroboh atau kurang hati-hati'),
(81,'PDP(Pendidikan dan Pembelajaran)','Cara guru menyajikan pelajaran terlalu kaku dan atau membosankan dan/atau'),
(82,'PDP(Pendidikan dan Pembelajaran)','Guru kurang bersahabat dan/atau membimbing siswa'),
(83,'PDP(Pendidikan dan Pembelajaran)','Mengalami masalah karena disiplin yang diterapkan oleh guru'),
(84,'PDP(Pendidikan dan Pembelajaran)','Dirugikan karena dalam menilai kemajuan atau keberhasilan siswaguru    kurang objektif'),
(85,'PDP(Pendidikan dan Pembelajaran)','Guru kurang memberikan tanggung jawab kepada siswa'),
(86,'PDP(Pendidikan dan Pembelajaran)','Guru kurang adil atau pilih kasih'),
(87,'PDP(Pendidikan dan Pembelajaran)','Ingin dekat dengan guru'),
(88,'PDP(Pendidikan dan Pembelajaran)','Guru kurang memperhatikan kebutuhan dan/atau keadaan siswa'),
(89,'PDP(Pendidikan dan Pembelajaran)','Mendapatkan perhatian khusus dari guru'),
(90,'PDP(Pendidikan dan Pembelajaran)','Dalam memberi pelajaran dan/atau berhubungan dengan siswa sikap dan/atau tindakan guru senang berubah-ubah sehingga membingungkan siswa'),
(91,'DPI(Diri Pribadi)','Sering murung dan/atau merasa tidak bahagia'),
(92,'DPI(Diri Pribadi)','Mengalami kerugian atau kesulitan karena terlampau hati-hati'),
(93,'DPI(Diri Pribadi)','Kurang serius menghadapi sesuatu yang penting'),
(94,'DPI(Diri Pribadi)','Merasa hidup kurang berarti'),
(95,'DPI(Diri Pribadi)','Sering gagal dan/atau mudah patah semangat'),
(96,'PDP(Pendidikan dan Pembelajaran)','Khawatir akan dipaksa melanjutkan pelajaran setamat sekolah ini'),
(97,'PDP(Pendidikan dan Pembelajaran)','Kekurangan informasi tentang pendidikan lanjutan yang dapat dimasuki setamat sekolah ini'),
(98,'PDP(Pendidikan dan Pembelajaran)','Ragu tentang kemanfaatan pendidikan lanjutan setamat sekolah ini'),
(99,'PDP(Pendidikan dan Pembelajaran)','Khawatir tidak mampu melanjutkan pelajaran setamat dari sekolah ini dan/atau terlalu memikirkan pendidikan lanjutan setamat sekolah ini'),
(100,'PDP(Pendidikan dan Pembelajaran)','Ragu apakah sekolah sekarang ini mampu memberikan modal yang kuat bagi para siswanya untuk menempuh pendidikan lebih lanjut'),
(101,'PDP(Pendidikan dan Pembelajaran)','Khawatir tidak tersedia biaya untuk melanjutkan pekerjaan setamat sekolah ini'),
(102,'PDP(Pendidikan dan Pembelajaran)','Tidak dapat mengambil keputusan tentang apakah akan mencari pekerjaan atau melanjutkan pelajaran setamat sekolah ini'),
(103,'PDP(Pendidikan dan Pembelajaran)','Khawatir tuntutan dan proses pendidikan lanjutan setamat sekolah ini sangat berat'),
(104,'PDP(Pendidikan dan Pembelajaran)','Terdapat pertentangan pendapat dengan orang tua dan/atau anggota keluarga lain tentang rencana melanjutkan pelajaran setamat sekolah ni'),
(105,'PDP(Pendidikan dan Pembelajaran)','Khawatir tidak bersaing dalam upaya memasuki pendidikan lanjutan setamat sekolah ini'),
(106,'DPI(Diri Pribadi)','Mudah gentar atau khawatir dalam menghadapi dan/atau mengemukakan sesuatu'),
(107,'DPI(Diri Pribadi)','Penakut pemalu dan/atau mudah jadi bingung'),
(108,'DPI(Diri Pribadi)','Keras kepala atau sukar mengubah pendapat sendiri meskipun kata orang lain pendapat itu salah'),
(109,'DPI(Diri Pribadi)','Takut mencoba sesuatu yang baru'),
(110,'DPI(Diri Pribadi)','Mudah marah atau tidak mampu mengendalikan diri'),
(111,'ANM(Agama dan Nilai Moral)','Mengalami masalah untuk pergi ke tempat peribadatan'),
(112,'ANM(Agama dan Nilai Moral)','Mempunyai pandangan atau kebiasaan yang tidak sesuai dengan kaidah-kaidah agama'),
(113,'ANM(Agama dan Nilai Moral)','Tidak mampu melaksanakan tuntutan keagamaan dan/atau khawatir tidak mampu menghindari larangan yang ditentukan agama'),
(114,'ANM(Agama dan Nilai Moral)','Kurang menyukai pembicaraan tentang agama'),
(115,'ANM(Agama dan Nilai Moral)','Ragu dan ingin memperoleh penjelasan lebih banyak tentang kaidah-kaidah agama'),
(116,'ANM(Agama dan Nilai Moral)','Mengalami kesulitan dalam mendalami agama'),
(117,'ANM(Agama dan Nilai Moral)','Tidak memiliki kecakapan dan sarana untuk melaksanakan'),
(118,'ANM(Agama dan Nilai Moral)','Mengalami masalah karena membandingkan agama yang satu dengan yang lain'),
(119,'ANM(Agama dan Nilai Moral)','Bermasalah karena anggota keluarga tidak seagama'),
(120,'ANM(Agama dan Nilai Moral)','Belum menjalankan ibadah agama sebagai diharapkan'),
(121,'DPI(Diri Pribadi)','Merasa kesepian dan/atau takut ditinggal sendiri'),
(122,'DPI(Diri Pribadi)','Sering bertingkah laku bertindak atau bersikap kekanak-kanakan'),
(123,'DPI(Diri Pribadi)','Rendah diri atau kurang percaya diri'),
(126,'ANM(Agama dan Nilai Moral)','Berkata dusta atau berbuat tidak jujur untuk tujuan-tujuan tertentu seperti membohongi teman berlaku curang dalam ujian'),
(127,'ANM(Agama dan Nilai Moral)','Kurang mengetahui hal-hal menurut orang lain dianggap baik atau buruk benar atau salah'),
(128,'ANM(Agama dan Nilai Moral)','Tidak dapat mengambil keputusan tentang sesuatu karena kurang memahami baik buruknya atau benar salahnya sesuatu itu'),
(129,'ANM(Agama dan Nilai Moral)','Merasa terganggu oleh kesalahan atau keburukan orang lain'),
(130,'ANM(Agama dan Nilai Moral)','Tidak mengetahui cara-cara yang tepat untuk mengatakan kepada orang lain tentang sesuatu yang baik atau buruk benar atau salah'),
(131,'ANM(Agama dan Nilai Moral)','Khawatir atau merasa ketakutan akan akibat perbuatan melanggar kaidah-kaidah agama'),
(132,'ANM(Agama dan Nilai Moral)','Kurang menyukai pembicaraan yang dilontarkan di tempat peribadatan'),
(133,'ANM(Agama dan Nilai Moral)','Kurang taat atau kurang khusyuk'),
(134,'ANM(Agama dan Nilai Moral)','Mengalami masalah karena memiliki pandangan dan/atau sikap keagamaan yang cenderung fanatik atau berprasangka'),
(135,'ANM(Agama dan Nilai Moral)','Meragukan manfaat ibadah dan/atau upacara keagamaan'),
(136,'HSO(Hubungan Sosial)','Tidak menyukai atau disukai seseorang'),
(137,'HSO(Hubungan Sosial)','Merasa diperhatikan atau dibicarakan atau di perolokkan orang lain'),
(138,'HSO(Hubungan Sosial)','mengalami masalah karena ingin lebih terkenal atau lebih menarik atau lebih menyenangkan bagi orang lain'),
(139,'HSO(Hubungan Sosial)','mempunyai kawan yang kurang disukai orang lain'),
(140,'HSO(Hubungan Sosial)','tidak mempunyai kawan akrab hubungan sosial terbatas atau terisolir'),
(141,'ANM(Agama dan Nilai Moral)','Merasa terganggu karena melakukan sesuatu yang menjadikan orang lain tidak senang'),
(142,'ANM(Agama dan Nilai Moral)','Terlanjur berbicara bertindak atau bersikap tidak layak kepada orang tua dan/atau orang lain'),
(143,'ANM(Agama dan Nilai Moral)','Senang ditegur karena dianggap melakukan kesalahan pelanggaran atau sesuatu yang tidak layak'),
(144,'ANM(Agama dan Nilai Moral)','Mengalami masalah karena berbohong atau berkata tidak layak meskipun sebenarnya dengan maksud sekedar mengolok-olok atau menimbulkan suasana gembira'),
(145,'ANM(Agama dan Nilai Moral)','Tidak melakukan sesuatu yang sesungguhnya perlu dilakukan'),
(146,'ANM(Agama dan Nilai Moral)','Takut dipersalahkan karena melanggar adat'),
(147,'ANM(Agama dan Nilai Moral)','Mengalami masalah karena memiliki kebiasaan yang berbeda dari orang lain'),
(148,'ANM(Agama dan Nilai Moral)','Tertampar melakukan sesuatu perbuatan yang salah atau melanggar nilai-nilai moral atau adat'),
(149,'ANM(Agama dan Nilai Moral)','Merasa bersalah karena terpaksa mengingkari janji'),
(150,'ANM(Agama dan Nilai Moral)','Mengalami persoalan karena berbeda pendapat tentang suatu aturan dalam adat'),
(151,'HSO(Hubungan Sosial)','Kurang peduli terhadap orang lain'),
(152,'HSO(Hubungan Sosial)','Rapuh dalam berteman'),
(153,'HSO(Hubungan Sosial)','Merasa tidak dianggap penting diremehkan atau dikecam oleh orang lain'),
(154,'HSO(Hubungan Sosial)','Mengalami masalah dengan orang lain karena kurang peduli terhadap diri sendiri'),
(155,'HSO(Hubungan Sosial)','Canggung dan/atau tidak lancar berkomunikasi dengan orang lain'),
(156,'HMP(Hubungan Muda-mudi dan Perkawinan)','Membutuhkan keterangan tentang seks pacaran dan/atau perkawinan'),
(157,'HMP(Hubungan Muda-mudi dan Perkawinan)','Mengalami masalah karena malu dan kurang terbuka dalam membicarakan soal seks pacar dan/atau jodoh'),
(158,'HMP(Hubungan Muda-mudi dan Perkawinan)','Khawatir tidak mendapatkan pacar atau jodoh yang baik/cocok'),
(159,'HMP(Hubungan Muda-mudi dan Perkawinan)','Terlalu memikirkan tentang seks percintaan pacaran atau perkawinan'),
(160,'HMP(Hubungan Muda-mudi dan Perkawinan)','Mengalami masalah karena dilarang atau merasa tidak patut berpacaran'),
(161,'KHK(Keadaan dan Hubungan dalam Keluarga)','Bermasalah karena kedua orang tua hidup berpisah atau cerai'),
(162,'KHK(Keadaan dan Hubungan dalam Keluarga)','Mengalami masalah karena ayah dan/atau ibu kandung telah meninggal'),
(163,'KHK(Keadaan dan Hubungan dalam Keluarga)','Mengkhawatirkan kondisi kesehatan anggota keluarga'),
(164,'KHK(Keadaan dan Hubungan dalam Keluarga)','Mengalami masalah karena keadaan dan perlengkapan tempat tinggal dan/atau rumah orang tua kurang memadai'),
(165,'KHK(Keadaan dan Hubungan dalam Keluarga)','Mengkhawatirkan kondisi orang tua yang bekerja terlalu berat'),
(166,'HSO(Hubungan Sosial)','Tidak lincah dan kurang mengetahui tentang tata krama pergaulan'),
(167,'HSO(Hubungan Sosial)','Kurang pandai memimpin dan/atau mudah dipengaruhi orang lain'),
(168,'HSO(Hubungan Sosial)','Sering membantah atau tidak menyukai sesuatu yang dikatakan/dirasakan orang lain atau dikatakan sombong'),
(169,'HSO(Hubungan Sosial)','Mudah tersinggung atau sakit hati dalam berhubungan dengan orang lain'),
(170,'HSO(Hubungan Sosial)','Lambat menjalin persahabatan'),
(171,'HMP(Hubungan Muda-mudi dan Perkawinan)','Kurang mendapatkan perhatian dari jenis kelamin lain atau pacar'),
(172,'HMP(Hubungan Muda-mudi dan Perkawinan)','Mengalami masalah karena ingin mempunyai pacar'),
(173,'HMP(Hubungan Muda-mudi dan Perkawinan)','Canggung dalam menghadapi jenis kelamin lain atau pacar'),
(174,'HMP(Hubungan Muda-mudi dan Perkawinan)','Suka mengendalikan dorongan seksual'),
(175,'HMP(Hubungan Muda-mudi dan Perkawinan)','Mengalami masalah dalam memilih teman akrab dari jenis kelamin lain atau pacar'),
(176,'KHK(Keadaan dan Hubungan dalam Keluarga)','Keluarga mengeluh tentang keadaan keuangan'),
(177,'KHK(Keadaan dan Hubungan dalam Keluarga)','Mengkhawatirkan keadaan orang tua yang bertempat tinggal jauh'),
(178,'KHK(Keadaan dan Hubungan dalam Keluarga)','Bermasalah karena ibu atau bapak akan kawin lagi'),
(179,'KHK(Keadaan dan Hubungan dalam Keluarga)','Khawatir tidak mampu memenuhi tuntutan atau harapan orang tua atau anggota keluarga lain'),
(180,'KHK(Keadaan dan Hubungan dalam Keluarga)','Membayangkan dan berpikir-pikir seandainya menjadi anak dari keluarga lain'),
(181,'EDK(Ekonomi & Keuangan)','Mengalami masalah karena kurang mampu berhemat atau berkemampuan keuangan sangat tidak mencukupi baik untuk keperluan sehari-hari maupun keperluan pekerjaan'),
(182,'EDK(Ekonomi & Keuangan)','Khawatir tidak mampu menamatkan sekolah ini atau putus sekolah harus segera bekerja'),
(183,'EDK(Ekonomi & Keuangan)','Mengalami masalah karena terlalu berhemat dan/atau ingin menabung'),
(184,'EDK(Ekonomi & Keuangan)','Kekurangan dalam keuangan menyebabkan dalam pengembangan diri terhambat'),
(185,'EDK(Ekonomi & Keuangan)','untuk memenuhi keuangan terpaksa sekolah sambil bekerja'),
(186,'HMP(Hubungan Muda-mudi dan Perkawinan)','Mengalami masalah karena takut atau sudah terlalu jauh berhubungan dengan jenis kelamin lain atau pacar'),
(187,'HMP(Hubungan Muda-mudi dan Perkawinan)','Bertepuk sebelah tangan dengan kawan akrab atau pacar'),
(188,'HMP(Hubungan Muda-mudi dan Perkawinan)','Takut ditinggalkan pacar atau patah hati cemburu atau cinta segi tiga'),
(189,'HMP(Hubungan Muda-mudi dan Perkawinan)','Khawatir akan dipaksa kawin'),
(190,'HMP(Hubungan Muda-mudi dan Perkawinan)','Mengalami masalah karena sering dan mudah jatuh cinta dan/atau rindu kepada pacar'),
(191,'KHK(Keadaan dan Hubungan dalam Keluarga)','Kurang mendapat perhatian dan pengertian dari orang tua dan/atau anggota keluarga'),
(192,'KHK(Keadaan dan Hubungan dalam Keluarga)','Mengalami kesulitan dengan bapak atau ibu tiri'),
(193,'KHK(Keadaan dan Hubungan dalam Keluarga)','Diperlakukan tidak adil oleh orang tua atau anggota keluarga lainnya'),
(194,'KHK(Keadaan dan Hubungan dalam Keluarga)','Khawatir akan terjadinya pertentangan atau percekcokan dalam keluarga'),
(195,'KHK(Keadaan dan Hubungan dalam Keluarga)','Hubungan dengan orang tua dan anggota keluarga kurang hangat kurang harmonis dan/atau kurang menggembirakan'),
(196,'EDK(Ekonomi & Keuangan)','Mengalami masalah karena ingin berpenghasilan sendiri'),
(197,'EDK(Ekonomi & Keuangan)','Berhutang yang cukup memberatkan'),
(198,'EDK(Ekonomi & Keuangan)','Besarnya uang yang diperoleh dan sumber-sumbernya tidak menentu'),
(199,'EDK(Ekonomi & Keuangan)','Khawatir akan kondisi keuangan orang tua atau orang yang menjadi sumber keuangan; jangan-jangan harus menjual atau mengandal harta keluarga'),
(200,'EDK(Ekonomi & Keuangan)','Mengalami masalah karena keuangan dikendalikan orang lain'),
(201,'WSG(Waktu Senggang)','Kekurangan waktu senggang seperti waktu istirahat waktu luang di sekolah ataupun di rumah waktu libur untuk bersikap santai dan/atau melakukan kegiatan yang menyenangkan atau rekreasi'),
(202,'WSG(Waktu Senggang)','Tidak diperkenankan atau kurang bebas dalam menggunakan waktu senggang yang tersedia untuk kegiatan yang disukai/diingini'),
(203,'WSG(Waktu Senggang)','Mengalami masalah untuk mengikuti kegiatan acara-acara gembira dan santai bersama kawan-kawan'),
(204,'WSG(Waktu Senggang)','Tidak mempunyai kawan akrab untuk bersama-sama mengisi waktu kosong'),
(205,'WSG(Waktu Senggang)','Mengalami masalah karena memikirkan atau membayangkan kesempatan waktu berlibur di tempat jauh indah tenang dan menyenangkan'),
(206,'KHK(Keadaan dan Hubungan dalam Keluarga)','Mengalami masalah karena menjadi anak tunggal anak sulung anak bungsu satu-satunya anak laki-laki satu-satunya atau satu-satunya anak perempuan'),
(207,'KHK(Keadaan dan Hubungan dalam Keluarga)','Hubungan kurang harmonis dengan kakak atau adik atau dengan anggota keluarga lainnya'),
(208,'KHK(Keadaan dan Hubungan dalam Keluarga)','Orang tua atau keluarga anggota lainnya terlalu berkuasa atau kurang memberi kebebasan'),
(209,'KHK(Keadaan dan Hubungan dalam Keluarga)','Dicurigai oleh orang tua atau anggota keluarga lainnya'),
(210,'KHK(Keadaan dan Hubungan dalam Keluarga)','Bermasalah karena dirumah orang tua tinggal orang atau anggota keluarga lainnya'),
(211,'EDK(Ekonomi & Keuangan)','Mengalami masalah karena membanding-bandingkan kondisi keuangan sendiri dengan kondisi keuangan orang lain'),
(212,'EDK(Ekonomi & Keuangan)','Kesulitan dalam mendapatkan penghasilan sendiri sambil sekolah'),
(213,'EDK(Ekonomi & Keuangan)','Mempertanyakan kemungkinan memperoleh beasiswa atau dana bantuan belajar lainnya'),
(214,'EDK(Ekonomi & Keuangan)','Orang lain menganggap pelit dan/atau tidak mau membantu kawan yang sedang mengalami kesulitan keuangan'),
(215,'EDK(Ekonomi & Keuangan)','Terpaksa berbagi pengeluaran keuangan dengan kakak atau adik atau anggota keluarga lain yang sama-sama membutuhkan biaya'),
(216,'WSG(Waktu Senggang)','Tidak mengetahui cara menggunakan waktu senggang yang ada'),
(217,'WSG(Waktu Senggang)','Kekurangan sarana seperti biaya kendaraan televisi buku-buku bacaan dan lain-lain'),
(218,'WSG(Waktu Senggang)','Mengalami masalah karena melaksanakan kegiatan atau acara yang kurang tepat dalam menggunakan waktu senggang'),
(219,'WSG(Waktu Senggang)','Mengalami masalah dalam menggunakan waktu senggang karena tidak memiliki keterampilan tertentu seperti bermain musik olah raga menari dan sebagainya'),
(220,'WSG(Waktu Senggang)','Kurang berminat atau tidak ada hal yang menarik dalam memanfaatkan waktu senggang yang tersedia'),
(221,'KHK(Keadaan dan Hubungan dalam Keluarga)','Tinggal di lingkungan keluarga atau tetangga yang kurang menyenangkan'),
(222,'KHK(Keadaan dan Hubungan dalam Keluarga)','Tidak sependapat dengan orang tua atau anggota keluarga tentang sesuatu yang direncanakan'),
(223,'KHK(Keadaan dan Hubungan dalam Keluarga)','Orang tua kurang senang kawan-kawan datang kerumah'),
(224,'KHK(Keadaan dan Hubungan dalam Keluarga)','Mengalami masalah karena rindu dan ingin bertemu dengan orang tua dan/atau anggota keluarga lainnya'),
(225,'KHK(Keadaan dan Hubungan dalam Keluarga)','Tidak betah dan ingin meninggalkan rumah karena keadaanya sangat tidak menyenangkan');
/*!40000 ALTER TABLE `soal_masalah` ENABLE KEYS */;
UNLOCK TABLES;
commit;

--
-- Table structure for table `users`
--

DROP TABLE IF EXISTS `users`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `users` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `nama` varchar(255) NOT NULL,
  `role_id` int(11) NOT NULL,
  `password` varchar(255) NOT NULL,
  `token` varchar(255) DEFAULT NULL,
  `kelas_id` int(11) DEFAULT NULL,
  `created_at` timestamp NULL DEFAULT current_timestamp(),
  `is_submited` tinyint(1) DEFAULT 1,
  `is_bk_counselor` tinyint(1) DEFAULT 0,
  `bk_specialization` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `role_id` (`role_id`),
  KEY `kelas_id` (`kelas_id`),
  KEY `idx_users_token` (`token`),
  KEY `idx_users_nama` (`nama`),
  CONSTRAINT `users_ibfk_1` FOREIGN KEY (`role_id`) REFERENCES `roles` (`id`),
  CONSTRAINT `users_ibfk_2` FOREIGN KEY (`kelas_id`) REFERENCES `kelas` (`id`) ON DELETE SET NULL
) ENGINE=InnoDB AUTO_INCREMENT=495 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `users`
--

LOCK TABLES `users` WRITE;
/*!40000 ALTER TABLE `users` DISABLE KEYS */;
set autocommit=0;
INSERT INTO `users` VALUES
(254,'Admin',1,'ggmu','217d457f4acb3f7b7ce5c0ab973c6d815d9f2d86e9739c32a79dec34aeebea5b',NULL,'2025-10-30 00:46:14',0,0,NULL),
(255,'BK',2,'ggmu','247fa7263d1f4b684bfaf5e9feb6f44d38ab5c8dd883cd6602f6adfff4d1d1b2',NULL,'2025-10-30 00:46:23',0,1,NULL);
/*!40000 ALTER TABLE `users` ENABLE KEYS */;
UNLOCK TABLES;
commit;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*M!100616 SET NOTE_VERBOSITY=@OLD_NOTE_VERBOSITY */;

-- Dump completed on 2025-10-30 13:20:46
