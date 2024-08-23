package com.example.practice_kotlin.design_patterns.builder

enum class Engine { ELECTRIC, HYBRID, DIESEL, PETROL, HYDROGEN, LNG }

enum class Camera { FRONT, REAR, BOTH, NONE }
enum class Smartphone { CarPlay, AndroidAuto, BOTH, NONE }
enum class ADAS { AdaptiveControl, Copilot, BOTH, NONE }


class CarConfiguration private constructor(
    // mandatory
    val id: Int,
    val brand: String,
    val engine: Engine,
    // optionals
    val camera: Camera?,
    val smartphone: Smartphone?,
    val adas: ADAS?
) {

    companion object {
        class Builder(var id: Int, var brand: String, var engine: Engine) {

            // optionals
            var camera: Camera? = null
            var smartphone: Smartphone? = null
            var adas: ADAS? = null

            // For aggregated updates - initialize the builder with previous configuration
            constructor (car: CarConfiguration) : this(car.id, car.brand, car.engine) {
                camera = car.camera
                smartphone = car.smartphone
                adas = car.adas
            }

            fun build() = CarConfiguration(id, brand, engine, camera, smartphone, adas)

            // For aggregated updates - the new (partial) configuration that will be applied on top of the current configuration
            fun update(car: CarConfiguration) = apply {
                id = car.id
                brand = car.brand
                engine = car.engine

                car.camera?.let { camera = it }
                car.smartphone?.let { smartphone = it }
                car.adas?.let { adas = it }
            }
        }

        /**
         * Helper methods, for building the outer class instance on the fly
         *
         * @param block This is lambda that extends Builder (placeholder for anonymous Extension Function) on the fly - Builder is receiver,
         * which means the callable can access any non-private (public) properties of the
         * receiver (all of them) through "this", in order to properly configure the outer class
         */
        inline fun build(id: Int, brand: String, engine: Engine, block: Builder.() -> Unit) =
            Builder(id, brand, engine).apply(block).build()

        // For aggregated updates
        inline fun build(currConfig: CarConfiguration, block: Builder.() -> Unit) =
            Builder(currConfig).apply(block).build()

        fun build(currConfig: CarConfiguration, newConfig: CarConfiguration) =
            Builder(currConfig).update(newConfig).build()
    }

    override fun toString(): String {
        return "[id=$id, brand=$brand, engine=$engine, camera=$camera, smartphone=$smartphone, adas=$adas]"
    }
}


