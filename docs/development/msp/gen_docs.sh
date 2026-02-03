INAV_MAIN_PATH="../../../src/main"

echo "###########"
echo get_all_inav_enums_h.py
python get_all_inav_enums_h.py --inav-root "$INAV_MAIN_PATH"

echo "###########"
echo "msp_messages.json checksum"
actual="$(md5sum msp_messages.json | awk '{print $1}')"
expected="$(awk '{print $1}' msp_messages.checksum)"
echo "Hash:" $actual
if [[ "$actual" != "$expected" ]]; then
  n="$(cat rev)"
  echo $((n + 1)) > rev
  echo "File changed, incrementing revision"
  echo $actual > msp_messages.checksum
fi

echo "###########"
echo gen_msp_md.py
python gen_msp_md.py

echo "###########"
echo gen_enum_md.py
python gen_enum_md.py
rm all_enums.h
read -n 1 -s -r -p "Press any key to continue"
