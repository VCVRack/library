# Tests

## Jasmine

[Jasmine Intro](https://jasmine.github.io/2.8/introduction.html)

To run all tests:
```
npm test
```

To run one set of tests:
```
npm test spec/basic.tests.spec.js
```

By default the zip tests only run on manifest files which have changed.

To force the zip tests to run for certain manifest files:
```
TEST_MANIFEST_ZIPS=plugins/JW-Modules.json npm test spec/zip.tests.spec.js
```

## Schema Validation

Our schema is [here](./manifest.json)

We use [ajv](https://github.com/epoberezkin/ajv) to validate manifests against that schema.


## Virus Total

*Currently Disabled because they are slow and annoying*

**Rate Limit: 4 per minute**

`VT_API_KEY` env var is required and already set in travisci settings

[Public API](https://www.virustotal.com/en/documentation/public-api/v2/)


