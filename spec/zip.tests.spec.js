const TEMP_DIR = ".tmp-zips/";
const fs = require("fs");
const request = require("request");
const { execSync } = require('child_process');
const AdmZip = require('adm-zip'); //https://github.com/cthackers/adm-zip
const hashFiles = require('hash-files'); //https://github.com/mac-/hash-files
const manifestsToTest = getManifestsThatApply();

if(manifestsToTest.length === 0){
    console.log("No manifest zip files to test.");
    process.exit(0);
}

//virus total stuff
const VIRUS_TOTAL_ENABLED = false;
const vt = require("node-virustotal"); //https://github.com/natewatson999/node-virustotal
const con = vt.MakePublicConnection();
if(VIRUS_TOTAL_ENABLED){
    con.setKey(process.env.VT_API_KEY);
    con.setDelay(15000);
    jasmine.DEFAULT_TIMEOUT_INTERVAL = manifestsToTest.length * 4 * con.getDelay();
}

describe("zips", function() {
    
    beforeEach(()=>{
        execSync(`mkdir -p ${TEMP_DIR}`)
    });
    
    afterEach(()=>{
        execSync(`rm ${TEMP_DIR}/*.zip`)
        fs.rmdirSync(TEMP_DIR);
    });

    const testZipsInMF = (filePath) => {
        it("valid zip files in manifest", function(done) {
            const mfObj = JSON.parse(fs.readFileSync(filePath, 'utf8'));
            if(mfObj.downloads){
                const urlsChecked = [];
                let lastSha256;
                ['win', 'lin', 'mac'].map(os => {
                    const osObj = mfObj.downloads[os];
                    if(osObj && osObj.download && osObj.sha256){
                        const zipUrl = osObj.download;
                        if(urlsChecked.includes(zipUrl)){
                            if(lastSha256 !== osObj.sha256){
                                fail("SHA256 should be the same if the download URL is the same");
                            }
                        } else {
                            urlsChecked.push(zipUrl);
                            lastSha256 = osObj.sha256;
                            testOneZip(mfObj.slug, osObj, done);
                        }
                    }
                });
            }
        });
    };

    manifestsToTest.map(testZipsInMF);
});

function testOneZip(expectedRootDir, osObj, done) {
    const urlParts = osObj.download.split('/');
    const zipName = urlParts[urlParts.length - 1].split('\?')[0];
    request(osObj.download).pipe(fs.createWriteStream(TEMP_DIR+zipName)).on('finish', ()=>{

        console.log(`Downloaded ${TEMP_DIR+zipName}`);
        const zip = new AdmZip(TEMP_DIR+zipName);
        const zipEntries = zip.getEntries();
        // zipEntries.map(ze=>console.log(ze.toString()));

        const slugDirFound = zipEntries.find(ze => ze.isDirectory && 
            (ze.entryName === expectedRootDir+'/' || ze.entryName === expectedRootDir+'\\')
        );
        if(!slugDirFound){
            fail(`Zip should have one dir named ${expectedRootDir}`);
        }

        const invalidEntry = zipEntries.find(ze => !ze.entryName.startsWith(slugDirFound.entryName));
        if(invalidEntry){
            fail(`Zip entries should all be under a dir named ${expectedRootDir} but this entry was found: ${invalidEntry.entryName}`);
        }

        if(VIRUS_TOTAL_ENABLED){
            con.FileEvaluation(zipName, "application/zip", fs.readFileSync(TEMP_DIR+zipName), function(data) {
                console.log(data);
                if(osObj.sha256 !== data.sha256){
                    throw new Error(`Invalid sha256 value.  manifest:${osObj.sha256} virustotal:${data.sha256}`);
                }
                if(data.positives > 2){
                    throw new Error(`Too many positives from virustotal.`);   
                }
                done();
            }, function(err) {
                if (err){ throw err; }
                done();
            });
        } else {
            hashFiles({files:[TEMP_DIR+zipName], algorithm:'sha256'}, function(error, hash) {
                if(osObj.sha256 !== hash){
                    throw new Error(`Invalid sha256 value.  manifest:${osObj.sha256} hash:${hash}`);
                }
                done();
            });
        }
    });
}

function getManifestsThatApply(){
    let paths = "";
    if(process.env.TEST_MANIFEST_ZIPS){
         paths = process.env.TEST_MANIFEST_ZIPS;
    } else {
        paths = execSync('git diff -w --stat --name-only origin/master -- plugins/', {encoding:'utf8'});
    }
    return paths.trim().split('\n').filter(s=>s.trim() !== '');
}

