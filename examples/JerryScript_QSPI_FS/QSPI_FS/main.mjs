import { exported_value } from "/js/modules/module.mjs"
import { getFeatureDetails } from "/js/modules/module_2.mjs"

print("module.mjs >> ", exported_value);
print("module_2.mjs >> ", getFeatureDetails());

print("main.mjs >> ", "Hello from FS");
