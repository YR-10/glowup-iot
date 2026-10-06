from pathlib import Path
from html import escape

OUT = Path(__file__).parent
s = []
def add(v): s.append(v)
def rect(x,y,w,h,fill='#ffffff',stroke='#d6e1eb',r=14):
    add(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{r}" fill="{fill}" stroke="{stroke}" stroke-width="2"/>')
def text(x,y,t,size=22,color='#18334c',weight=400,anchor='start'):
    add(f'<text x="{x}" y="{y}" font-family="Arial, sans-serif" font-size="{size}" fill="{color}" font-weight="{weight}" text-anchor="{anchor}">{escape(t)}</text>')
def wire(points,color='#263746',width=4):
    add(f'<polyline points="{" ".join(f"{x},{y}" for x,y in points)}" fill="none" stroke="{color}" stroke-width="{width}" stroke-linejoin="round" stroke-linecap="round"/>')
def dot(x,y,color='#263746'):
    add(f'<circle cx="{x}" cy="{y}" r="5" fill="{color}"/>')
def pin(x,y,label,side='right',color='#263746'):
    add(f'<circle cx="{x}" cy="{y}" r="6" fill="white" stroke="{color}" stroke-width="3"/>')
    text(x+14 if side=='right' else x-14,y+7,label,20,color,600,'start' if side=='right' else 'end')
def tag(x,y,label,color):
    w=max(66,len(label)*12+24)
    rect(x,y-22,w,32,'#ffffff',color,6); text(x+w/2,y+1,label,18,color,700,'middle')
def block(x,y,w,h,title,subtitle='',fill='#edf4fa'):
    rect(x,y,w,h,fill); text(x+w/2,y+35,title,24,'#18334c',700,'middle')
    if subtitle: text(x+w/2,y+62,subtitle,17,'#566d80',400,'middle')
R='#ce343e'; Y='#b98700'; O='#d46a00'; G='#21894e'; B='#2471c8'; P='#8850bc'; C='#008e9c'; K='#263746'; BR='#865b3b'
add('<svg xmlns="http://www.w3.org/2000/svg" width="2000" height="1680" viewBox="0 0 2000 1680">')
rect(0,0,2000,1680,'#eef3f7','#eef3f7',0)
rect(0,0,2000,130,'#143653','#143653',0)
text(50,57,'SMART LIGHT · ESP8266',40,'white',700)
text(50,99,'Diagram koneksi revisi • AUTO melalui LDR + MANUAL melalui web • LED 12 V langsung dari adaptor',23,'#cedfeb')
tag(1600,60,'TANPA XL6009','#31755a')

rect(35,155,1930,500)
text(65,199,'01 / DAYA & SAKELAR LED',25,'#18334c',700)
text(65,229,'Ikuti nama terminal. Posisi pin pada ilustrasi bukan urutan kaki fisik komponen.',19,'#566d80')
block(75,300,235,180,'ADAPTOR','12 V DC · 2 A')
block(475,300,265,180,'LM2596','Step-down · set 5,0 V')
pin(310,390,'+12 V','left',R); pin(310,445,'GND','left',K)
pin(475,390,'IN+','right',R); pin(475,445,'IN−','right',K)
pin(740,390,'OUT+','left',Y); pin(740,445,'OUT−','left',K)
wire([(310,390),(475,390)],R)
wire([(375,390),(375,267),(1710,267),(1710,305)],R); dot(375,390,R)
text(970,254,'+12 V langsung ke LED',20,R,700)
wire([(740,390),(940,390)],Y); tag(845,390,'+5V',Y)
text(785,432,'Ke VIN NodeMCU, LCD VCC',18)
text(785,460,'dan HV level shifter',18)
block(1550,305,335,100,'LED STRIP 12 V','Beban DC yang mendukung PWM','#fff7d7')
pin(1710,305,'+','right',R); pin(1710,405,'−','right',BR)
wire([(1710,405),(1710,460)],BR)
rect(1645,460,130,110,'#e8edf2')
text(1710,491,'N-MOSFET',17,K,700,'middle')
pin(1710,460,'D','right',BR); pin(1645,520,'G','right',G); pin(1710,570,'S','right',K)
wire([(1170,520),(1280,520)],G); tag(1110,520,'PWM D5',G)
rect(1280,506,85,28,'#fff5cf','#b98700',5); text(1322,527,'220 Ω',17,K,600,'middle')
wire([(1365,520),(1645,520)],G); dot(1465,520,G)
wire([(1465,520),(1465,552)],G)
rect(1451,552,28,52,'#fff5cf','#b98700',4); text(1500,586,'10 kΩ',20,K,600)
wire([(1465,604),(1465,620)],K)
wire([(310,445),(340,445),(340,620),(1880,620)],K,5)
wire([(475,445),(445,445),(445,620)],K); dot(445,620)
wire([(740,445),(760,445),(760,620)],K); dot(760,620)
wire([(1710,570),(1710,620)],K,5); dot(1710,620); dot(1465,620)
tag(795,621,'GND COMMON',K)
text(1100,347,'MOSFET: pilih RDS(on) yang dijamin',19)
text(1100,374,'pada VGS ≤ 3,3 V; sesuaikan arus & termal.',19)
text(1100,408,'G = Gate   D = Drain   S = Source',19,'#566d80')

rect(35,680,1930,555)
text(65,725,'02 / SENSOR, NODEMCU & LCD',25,'#18334c',700)
text(65,755,'Label rail yang sama saling terhubung: +5V, +3V3, GND. Semua GND menuju bus pada panel 01.',19,'#566d80')
block(75,845,240,270,'MODUL LDR','4 pin · analog','#e9f6f6')
block(650,810,325,350,'NodeMCU LoLin','ESP8266','#eaf0f6')
block(1160,820,310,305,'LEVEL SHIFTER','I2C bidirectional · 2 kanal','#f0eafb')
block(1660,845,255,245,'LCD 16 × 2','Backpack I2C · 5 V','#eaf6ec')
rect(1680,918,215,76,'#173956','#173956',7)
text(1694,949,'Smart Light',22,'#c7f6d9',600)
text(1694,979,'LDR:65% AUTO',21,'#c7f6d9')

pin(315,935,'AO','left',C); pin(315,995,'VCC','left',O)
pin(315,1050,'GND','left',K); pin(75,1080,'DO: NC','right','#8594a0')
pin(650,935,'A0','right',C); pin(650,995,'3V3','right',O)
wire([(315,935),(365,935)],C)
rect(365,894,235,83,'#fff7e8','#d5a54b',8)
text(482,918,'PENYESUAIAN A0',17,'#9b6119',700,'middle')
text(482,942,'Pembagi bila diperlukan',17,'#9b6119',400,'middle')
text(482,965,'Verifikasi batas board',17,'#9b6119',400,'middle')
wire([(600,935),(650,935)],C)
wire([(315,995),(650,995)],O); tag(425,995,'+3V3',O)
wire([(315,1050),(380,1050)],K); tag(380,1050,'GND',K)
pin(650,1100,'VIN','right',Y); wire([(530,1100),(650,1100)],Y); tag(480,1100,'+5V',Y)
pin(812,1160,'GND','right',K); wire([(812,1160),(812,1200)],K); tag(779,1200,'GND',K)
pin(975,935,'D2 / GPIO4','left',B); pin(975,995,'D1 / GPIO5','left',P)
pin(975,1100,'D5 / GPIO14','left',G)
wire([(975,1100),(1055,1100),(1055,1185)],G); tag(998,1204,'PWM D5',G)
pin(1160,935,'LV1','right',B); pin(1160,995,'LV2','right',P)
pin(1470,935,'HV1','left',B); pin(1470,995,'HV2','left',P)
wire([(975,935),(1160,935)],B); wire([(975,995),(1160,995)],P)
pin(1660,1030,'SDA','right',B); pin(1660,1070,'SCL','right',P)
wire([(1470,935),(1540,935),(1540,1030),(1660,1030)],B)
wire([(1470,995),(1515,995),(1515,1105),(1615,1105),(1615,1070),(1660,1070)],P)
pin(1210,1125,'LV','right',O); pin(1320,1125,'HV','right',Y); pin(1430,1125,'GND','left',K)
wire([(1210,1125),(1210,1180)],O); tag(1170,1200,'+3V3',O)
wire([(1320,1125),(1320,1180)],Y); tag(1290,1200,'+5V',Y)
wire([(1430,1125),(1430,1180)],K); tag(1400,1200,'GND',K)
pin(1770,845,'VCC','right',Y); wire([(1770,845),(1770,802)],Y); tag(1735,802,'+5V',Y)
pin(1865,1090,'GND','left',K); wire([(1865,1090),(1865,1180)],K); tag(1830,1200,'GND',K)

rect(35,1260,780,370)
text(65,1304,'03 / WARNA & PIN',25,'#18334c',700)
legend=[('12 V dari adaptor',R),('5 V dari LM2596',Y),('3,3 V dari NodeMCU',O),('Common ground',K),('PWM D5 / GPIO14',G),('SDA D2 / GPIO4',B),('SCL D1 / GPIO5',P),('Analog LDR → A0',C),('LED negatif → Drain',BR)]
for i,(label,color) in enumerate(legend):
    col=i//5; row=i%5; x=65+col*385; y=1350+row*47
    wire([(x,y-6),(x+35,y-6)],color); text(x+50,y,label,19)
rect(840,1260,1125,370,'#fffaf0','#e4c785')
text(870,1304,'04 / SEBELUM DIRAKIT',25,'#825a19',700)
notes=[
'1. Set LM2596 ke 5,0 V dengan multimeter sebelum menghubungkan beban.',
'2. ADC chip ESP8266: 0–1 V. Batas A0 board bergantung pada pembagi internal.',
'3. LCD 5 V: gunakan level shifter I2C dengan pull-up pada masing-masing sisi.',
'4. Verifikasi MOSFET aktual, urutan kaki, arus LED, kabel, dan suhu kerja.',
'5. Jalur arus LED kembali ke ground daya; hindari melewati ground sensor.',
'6. Adaptor 2 A juga menyuplai elektronik: sisakan daya dan margin untuk sistem.'
]
for i,n in enumerate(notes): text(870,1348+i*39,n,20,'#674e26')
text(65,1660,'REVISI KONSEP • Sambungan fisik dan rating komponen belum diverifikasi • Titik bulat = sambungan listrik',19,'#566d80')
add('</svg>')
(OUT/'smart-light-revisi-v2.svg').write_text('\n'.join(s),encoding='utf-8')
print(OUT/'smart-light-revisi-v2.svg')
