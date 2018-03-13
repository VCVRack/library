---
layout: page
---

<h1>{{ page.name }}</h1>
<span>Added: version {{ page.add_version }}</span>
<p>
	<img style="margin: 4px; padding: 32px" src="{{ site.baseurl }}/screenshots/{{ page.key }}.png" align="left" hspace="25"/>
	{{ content }}
</p>
