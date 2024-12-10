npm i -g moxygen

# Moxygen will fail if doc/generated_doxygen_docs/docs.ms does not exist
mkdir -p doc/generated_doxygen_docs/docs.ms

moxygen --anchors --output doc/generated_doxygen_docs/docs.ms/api-docs.md doc/generated_doxygen_docs/xml
