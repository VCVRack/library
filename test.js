/*
 
check valid JSON
check valid SHA256
upload to VirusTotal and assert less than 3 or so engines return suspicious for each ZIP

ZIP must not contain a __MACOSX directory at root level.
ZIP must not be a tarbomb. Yes, I know the first is a subset of this one, but it's better to have more feedback.
We might even want to assert that the single folder at root level is named as the slug of the plugin. I'm for this.

schema validation

slug required
if we could have a "soft" warning for slugs matching /[a-zA-Z0-9_\-]/+, that'd be great, although I'd rather their slugs break this than change.
version required
version must begin with "0.5." (this will be changed with each version bump.)

*/



const fs = require("fs");
const request = require("request");
const { exec } = require('child_process');
const vt = require("node-virustotal");//https://github.com/natewatson999/node-virustotal
const con = vt.MakePublicConnection();
const archArr = ['win', 'lin', 'mac'];
const verbose = false;

//VT_API_KEY is set in travisci (or set when run locally)
con.setKey(process.env.VT_API_KEY);
if(verbose){console.log('key', con.getKey());}

//15 seconds between calls is required by virus total (4 per minute)
//https://www.virustotal.com/en/documentation/public-api/v2/
con.setDelay(15000);
if(verbose){console.log('delay', con.getDelay());}

//We always just check all JSON because it's easy.  
//However, we only want to check zip files of manifests that have changed.



//TODO FIX - travis doesn't have master
//           https://github.com/travis-ci/travis-ci/issues/6069
//           todo check diff to see if "download" or "sha256" changed 
exec('git diff -w --stat --name-only origin/master -- plugins/', (error, stdout, stderr) => {
  if (error) {
    console.error(`exec error: ${error}`);
    return;
  }
  const changedManifestFiles = stdout.trim().split('\n');
  testAllManifests(changedManifestFiles);
});

function testAllManifests(changedManifestFiles){
    if(verbose){console.log("changedManifestFiles", changedManifestFiles);}
    fs.readdir('plugins', function(err, files) {
        if (err){ throw err; }

        for (let index in files) {
            const filePath = `plugins/${files[index]}`;
            if(!filePath.toLowerCase().endsWith('.json')){
                throw new Error("manifests should have .json extension");
            }
            fs.readFile(filePath, 'utf8', (err, data) => {
                if (err){ throw err; }
                if(verbose){console.log("testing: ", filePath);}
                
                const shouldTestZip = changedManifestFiles.includes(filePath);
                testOneManifest(filePath, data, shouldTestZip);
            });
        }
    });
}

function testOneManifest(filePath, fileContent, shouldTestZip = false) {
    let manifestObj;
    try {
        manifestObj = JSON.parse(fileContent);
    } catch(err){
        console.error(`Invalid JSON: ${filePath}`);
        throw err;
    }

    if (manifestObj.downloads) {
        let zipUrlsChecked = [];
        let lastSha256;
        
        archArr.map(arch => {
            const archObj = manifestObj.downloads[arch];
            if (archObj && archObj.download) {
                if(zipUrlsChecked.includes(archObj.download)){
                    if(lastSha256 !== archObj.sha256){
                        throw new Error('SHA256 should be the same if the download URL is the same.');
                    }
                } else {
                    zipUrlsChecked.push(archObj.download);
                    lastSha256 = archObj.sha256;

                    if(shouldTestZip){
                        testOneArch(archObj);
                    } else {
                        if(verbose){console.log("not testing zip because the manifest hasn't changed: ", filePath);}
                    }
                }
            }
        });
    }
}

function testOneArch(archObj) {
    const urlParts = archObj.download.split('/');
    const zipName = urlParts[urlParts.length - 1].split('\?')[0];
    if(verbose){console.log(`Downloading ${archObj.download}`);}
    request(archObj.download).pipe(fs.createWriteStream(zipName)).on('finish', ()=>{
        con.FileEvaluation(zipName, "application/zip", fs.readFileSync(zipName), function(data) {
            console.log(data);
            if(archObj.sha256 !== data.sha256){
                throw new Error(`Invalid sha256 value.  manifest:${archObj.sha256} virustotal:${data.sha256}`);
            }
            if(data.positives > 2){
                throw new Error(`Too many positives from virustotal.`);   
            }
        }, function(err) {
            if (err){ throw err; }
        });
    });
}
