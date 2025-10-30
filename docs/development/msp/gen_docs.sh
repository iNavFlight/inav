echo "###########"
echo get_all_inav_enums_h.py
python get_all_inav_enums_h.py

echo "###########"
echo gen_msp_md.py
python gen_msp_md.py

echo "###########"
echo gen_enum_md.py
python gen_enum_md.py
rm all_enums.h
read -n 1 -s -r -p "Press any key to continue"