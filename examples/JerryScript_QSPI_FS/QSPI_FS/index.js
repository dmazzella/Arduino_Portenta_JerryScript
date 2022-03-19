import { exported_value } from "/js/modules/module.js"
import { getFeatureDetails } from "/js/modules/module_2.js"

print("module.js >> ", exported_value);
print("module_2.js >> ", getFeatureDetails());

print("index.js >> ", "Hello from FS");
