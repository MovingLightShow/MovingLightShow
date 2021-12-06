/* Moving Light Show remote control                          *
 * https://MovingLightShow.art - contact@movinglightshow.art *
 * Version 1.0.8.4                                           *
 * (c) 2020-2021 Showband Les Armourins                      */

// Names of the two caches used in this version of the service worker.
// Change to v2, etc. when you update any of the local resources, which will
// in turn trigger the install event again.
const PRECACHE = 'precache-1.0.8.4-1855';
const RUNTIME = 'runtime-1.0.8.4-1855';

// A list of local resources we always want to be cached.
const PRECACHE_URLS = [
  'index.html',
  './', // Alias for index.html
  // 'js/service-worker.js', // Don't cache the service worker !
  'js/crc8.js',
  'js/remote.js',
  'js/pwacompat.min.js',
  'css/remote.css',
  'css/bulma.min.css',
  'images/logo.png',
  'images/icons-192.png',
  'images/icons-512.png',
  'fontawesome/js/all.min.js',
  'fontawesome/css/all.min.css',
  'fontawesome/webfonts/fa-solid-900.woff2',
  'fontawesome/webfonts/fa-solid-900.woff',
  'fontawesome/webfonts/fa-solid-900.ttf',
  'fontawesome/webfonts/fa-solid-900.svg',
  'fontawesome/webfonts/fa-solid-900.eot',
  'fontawesome/webfonts/fa-regular-400.woff2',
  'fontawesome/webfonts/fa-regular-400.woff',
  'fontawesome/webfonts/fa-regular-400.ttf',
  'fontawesome/webfonts/fa-regular-400.svg',
  'fontawesome/webfonts/fa-regular-400.eot',
  'fontawesome/webfonts/fa-brands-400.woff2',
  'fontawesome/webfonts/fa-brands-400.woff',
  'fontawesome/webfonts/fa-brands-400.ttf',
  'fontawesome/webfonts/fa-brands-400.svg',
  'fontawesome/webfonts/fa-brands-400.eot'
];

// The install handler takes care of precaching the resources we always need.
self.addEventListener('install', event => {
  event.waitUntil(
    caches.open(PRECACHE)
      .then(cache => cache.addAll(PRECACHE_URLS))
      .then(self.skipWaiting())
  );
});

// The activate handler takes care of cleaning up old caches.
self.addEventListener('activate', event => {
  const currentCaches = [PRECACHE, RUNTIME];
  event.waitUntil(
    caches.keys().then(cacheNames => {
      return cacheNames.filter(cacheName => !currentCaches.includes(cacheName));
    }).then(cachesToDelete => {
      return Promise.all(cachesToDelete.map(cacheToDelete => {
        return caches.delete(cacheToDelete);
      }));
    }).then(() => self.clients.claim())
  );
});

// The fetch handler serves responses for same-origin resources from a cache.
// If no response is found, it populates the runtime cache with the response
// from the network before returning it to the page.
self.addEventListener('fetch', event => {
  // Skip cross-origin requests, like those for Google Analytics.
  if (event.request.url.startsWith(self.location.origin)) {
    event.respondWith(
      caches.match(event.request).then(cachedResponse => {
        if (cachedResponse) {
          return cachedResponse;
        }

        return caches.open(RUNTIME).then(cache => {
          return fetch(event.request).then(response => {
            // Put a copy of the response in the runtime cache.
            return cache.put(event.request, response.clone()).then(() => {
              return response;
            });
          });
        });
      })
    );
  }
});