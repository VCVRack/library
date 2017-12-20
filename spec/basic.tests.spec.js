const CUR_VER_PREFIX = '0.5';
const fs = require("fs");
const Ajv = require('ajv'); //https://github.com/epoberezkin/ajv
const ajv = new Ajv({});
const validate = ajv.compile(require('./manifest.json'));

describe("json", function() {

  it("manifest files should be valid against the schema", function(done) {
    fs.readdir('plugins', function(err, files) {
        if (err){ 
            fail("unable to read plugins dir"); 
        }

        for (let index in files) {
            const fileName = files[index];
            const filePath = `plugins/${fileName}`;
            if(!filePath.toLowerCase().endsWith('.json')){
                fail("manifests should have .json extension");
            }
        
            fs.readFile(filePath, 'utf8', (err, fileContent) => {
                if (err){ 
                    fail("unable to read manifest file"); 
                }
                
                let manifestObj;
                try {

                    const obj = JSON.parse(fileContent);
                    const valid = validate(obj);

                    if (!valid) {
                        validate.errors.map(e=>e.message+=` in ${filePath}`)
                        fail(validate.errors);
                    }

                    if(!(/^[a-zA-Z0-9_\-]*$/).test(obj.slug)){
                        fail(`slug does not match regex in ${filePath}`);
                    }

                    if(fileName.replace('.json','') !== obj.slug){
                        fail(`slug '${obj.slug}' does not match fileName: ${fileName}`);
                    }

                    if(obj.version && !obj.version.startsWith(CUR_VER_PREFIX)){
                        fail(`version '${obj.version}' must start with '${CUR_VER_PREFIX}'`);
                    }

                    done();
                } catch(err){
                    fail(`Invalid JSON: ${filePath}\n${err}`);
                }
            });
        }
    });
  });

});
