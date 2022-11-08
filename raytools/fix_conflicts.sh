while read file; do git checkout upstream/master "$file" && git add "$file"
done <files.txt && git rebase --continue

