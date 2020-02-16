import * as fs from 'fs';

const forbbidenFiles = [
    "all.js",
    "utils.js"
];

(async () => {
    const files = fs.readdirSync(__dirname).filter(file => {
        if (/^.*\.js$/.test(file) && !forbbidenFiles.includes(file))
            return true;
        return false;
    });
    for (const file of files) {
        await require(`./${file}`).run;
    }
})();