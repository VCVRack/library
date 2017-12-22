const CUR_VER_PREFIX = '0.5';
const MF_DIR = 'plugins';
const fs = require("fs");

const Ajv = require('ajv'); //https://github.com/epoberezkin/ajv
const ajv = new Ajv({allErrors:true});
const validate = ajv.compile(require('./manifest.json'));

describe("json", function() {

    let testMF = (fileName) => {
        it("manifest file should be valid", () => {
            try {
                const filePath = `${MF_DIR}/${fileName}`;
                if (!filePath.toLowerCase().endsWith('.json')) {
                    fail("manifests should have .json extension");
                }

                const fileContent = fs.readFileSync(filePath, 'utf8');
                const mfObj = JSON.parse(fileContent);
                const valid = validate(mfObj);
                if (!valid) {
                    validate.errors.map(e => e.message += ` in ${filePath}`)
                    fail(validate.errors);
                }

                if (!(/^[a-zA-Z0-9_\-]*$/).test(mfObj.slug)) {
                    fail(`slug does not match regex in ${filePath}`);
                }

                if (fileName.replace('.json', '') !== mfObj.slug) {
                    fail(`slug '${mfObj.slug}' does not match fileName: ${fileName}`);
                }

                if (mfObj.version && !mfObj.version.startsWith(CUR_VER_PREFIX)) {
                    fail(`version '${mfObj.version}' must start with '${CUR_VER_PREFIX}'`);
                }

            } catch(err){
                fail(`Error while trying to validate manifest: ${fileName}\n${err}`);
            }
        });
    };
    
    fs.readdirSync(MF_DIR).map(testMF);

});
