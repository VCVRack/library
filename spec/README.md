# Tests

## Jasmine

[Jasmine Intro](https://jasmine.github.io/2.8/introduction.html)

Tests only run on changed manifests files.

To run all tests:
```
npm test
```

To force the tests to run for certain manifest files:
```
TEST_MANIFEST=plugins/JW-Modules.json npm test
```

## Schema Validation

Our schema is [here](./manifest.json)

We use [ajv](https://github.com/epoberezkin/ajv) to validate manifests against that schema.


## Virus Total

*Currently Disabled because they are slow and annoying*

**Rate Limit: 4 per minute**

`VT_API_KEY` env var is required and already set in travisci settings

[Public API](https://www.virustotal.com/en/documentation/public-api/v2/)


