<?
/* ---------------------------------- */
//$TITLE=$m_pre_title."설정";
/* ---------------------------------- */
$a_empty_ssid = "SSID 필드는 반드시 입력해야 합니다.";
$a_invalid_ssid = "부적절한 문자가 SSID 필드에 있습니다. 체크하시길 바랍니다.";

$a_invalid_wep_128_wep_key = "키값이 부적절합니다. 키값은 13개 문자 또는 26개의 16진수이어야 합니다";
$a_invalid_wep_64_wep_key = "키값이 부적절합니다. 키값은 5개 문자 또는 10개의 16진수이어야 합니다";
$a_invalid_wep_128_ascii_wep_key = "키값이 부적절합니다. 키값은 반드시 13개의 문자이어야 합니다";
$a_invalid_wep_128_hex_wep_key = "키값이 부적절합니다. 키값은 반드시 26개의 16진수이어야 합니다";
$a_invalid_wep_64_ascii_wep_key = "키값이 부적절합니다. 키값은 반드시 5개의 문자이어야 합니다";
$a_invalid_wep_64_hex_wep_key = "키값이 부적절합니다. 키값은 반드시 10개의 16진수이어야 합니다";

$a_empty_defkey = "디폴트 WEP 키를 입력해야 합니다";
$a_valid_hex_char = "적합한 문자는 0~9, A~F 또는 a~f 입니다.";
$a_valid_asc_char = "적합한 문자는 ASCII 입니다.";

$a_invalid_radius_ip1		= "RADIUS 서버 1의 IP 주소가 잘못되었습니다.";
$a_invalid_radius_ip2		= "RADIUS 서버 2의 IP 주소가 잘못되었습니다.";
$a_invalid_radius_port1		= "RADIUS 서버 1의 포트가 잘못되었습니다.";
$a_invalid_radius_port2		= "RADIUS 서버 2의 포트가 잘못되었습니다.";
$a_empty_radius_sec1		= "RADIUS 서버 1의 공유 키(Shared Secret) 란이 공백이어서는 안됩니다. ";
$a_empty_radius_sec2		= "RADIUS 서버 2의 공유 키(Shared Secret) 란이 공백이어서는 안됩니다. ";
$a_invalid_radius_sec1		= "RADIUS 서버 1의 공유 키(Shared Secret) 값은 ASCII 문자여야 합니다. ";
$a_invalid_radius_sec2		= "RADIUS 서버 2의 공유 키(Shared Secret) 값은 ASCII 문자여야 합니다. ";
$a_invalid_passphrase_len = "패스프레이즈의 길이는 8~63이어야 합니다.";
$a_invalid_psk_len = "PSK의 길이는 64이어야 합니다.";
$a_psk_not_match = "재입력한 패스프레이즈가 서로 일치하지 않습니다";
$a_invalid_passphrase = "패스프레이즈의 길이는 8~63이어야 합니다.";
$a_invalid_psk = "PSK는 Hex이어야 합니다.";
$a_reset_wps_pin = "PIN을 디폴트로 리셋하시겠어요?";
$a_gen_new_wps_pin = "새로운 PIN을 만들겠습니까?";
$a_reset_wps_unconfig = "디바이스를 리셋하여 설정을 다 지우시겠습니까?";
$a_enable_wps_first = "WPS를 사용하고 있지 않습니다. \\\설정 저장\\\"을 클릭하여 WPS를 우선 사용하시길 바랍니다.";
$m_title_wireless_setting = "무선 네트워크 설정";
$m_enable_wireless = "무선 사용";
$m_wlan_name = "무선 네트워크 이름";
$m_wlan_name_comment = "(SSID라고 불리기도 함)";
$m_wlan_channel = "무선 채널";
$m_enable_auto_channel = "자동 채널 선택 사용";
$m_super_g = "수퍼 G 모드";
$m_super_g_without_turbo = "터보 없는 수퍼 G";
$m_super_g_with_d_turbo = "다이나믹 터보 수퍼 G";
$m_xr = "확장 범위 모드 사용";
$m_11g_only = "802.11g만 사용 모드";
$m_txrate = "전송률";
$m_mcrate = "멀티캐스트률";
$m_best_auto = "최적(자동)";
$m_mbps = "(Mbit/s)";
$m_wmm_enable = "WMM 사용";
$m_wlan_qos = "(무선 Qos)";
$m_enable_ap_hidden = "무선 숨김 사용";
$m_ap_hidden_comment = "(SSID 브로드캐스트로 불리기도 함)";
$m_title_wireless_security = "무선 보안 모드";
$m_security_mode = "보안 모드";
$m_disable_security = "무선 보안 사용안함(권장 안함)";
$m_enable_wep = "WEP 무선 보안 사용(기본)";
$m_wpa_security = "WPA만 지원 무선 보안 사용(강화)";
$m_wpa2_security = "WPA2만 지원 무선 보안 사용(강화)";
$m_wpa2_auto_security = "WPA/WPA2 무선 보안 사용(강화)";
$m_title_wep = "WEP";
$m_auth_type = "인증";
$m_open = "열기";
$m_shared_key = "공유키";
$m_wep_key_len = "WEP 인크립션";
$m_64bit_wep = "64Bit";
$m_128bit_wep = "128Bit";
$m_hex = "HEX";
$m_ascii = "ASCII";
$m_key_type = "키 타입";
$m_default_wep_key = "디폴트 WEP 키";
$m_wep_key = "WEP 키";
$m_wep64_hint_wording = "(5 ASCII 또는 10 HEX)";
$m_wep128_hint_wording = "(13 ASCII 또는 26 HEX)";

$m_title_wpa = "WPA 만";
$m_dsc_wpa = "WPA는 높은 등급의 암호와 인증을 필요로 합니다.";
$m_title_wpa2 = "WPA2 만";
$m_dsc_wpa2 = "WPA2는 높은 등급의 암호와 인증을 필요로 합니다.";
$m_title_wpa2_auto = "WPA/WPA2 만";
$m_dsc_wpa2_auto = "WPA/WPA2는 높은 등급의 암호와 인증을 필요로 합니다.";

$m_cipher_type = "암호화 타입";
$m_tkip = "TKIP";
$m_aes = "AES";
$m_tkipandaes = "자동(TKIP/AES)";
$m_psk_eap = "PSK / EAP";
$m_psk = "PSK";
$m_eap = "EAP";
$m_passphrase = "네트워크 키";
$m_confirm_passphrase = "네트워크 키 확인";
$m_psk_hint_wording = "(8~63 ASCII or 64 HEX)";

$m_8021x = "802.1X";
$m_radius1 = "RADIUS 서버";
$m_shared_sec = "공유키";
$m_tkipandaes = "자동(TKIP/AES)";
?>
