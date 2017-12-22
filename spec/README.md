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

To force the zip tests to run for certain manifest files:
```
TEST_MANIFEST_ZIPS=plugins/JW-Modules.json npm test
```

## Virus Total

**Rate Limit: 4 per minute**

`VT_API_KEY` env var is required and already set in travisci settings

[Public API](https://www.virustotal.com/en/documentation/public-api/v2/)


