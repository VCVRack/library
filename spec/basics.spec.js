//Jasmine Test - https://jasmine.github.io/2.8/introduction.html
const fs = require("fs");

describe("json", function() {
  
  it("manifest files should be valid parsable json", function(done) {
    fs.readdir('plugins', function(err, files) {
        if (err){ 
            fail("unable to read plugins dir"); 
        }

        for (let index in files) {
            const filePath = `plugins/${files[index]}`;
            if(!filePath.toLowerCase().endsWith('.json')){
                fail("manifests should have .json extension");
            }
        
            fs.readFile(filePath, 'utf8', (err, fileContent) => {
                if (err){ 
                    fail("unable to read manifest file"); 
                }
                
                let manifestObj;
                try {
                    manifestObj = JSON.parse(fileContent);
                    done();
                } catch(err){
                    fail(`Invalid JSON: ${filePath}\n${err}`);
                }
            });
        }
    });
  });

});
